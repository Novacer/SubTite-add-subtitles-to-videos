#include "subtitler/gui/auto_transcribe/tasks/transcribe_task.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QTemporaryFile>
#include <filesystem>
#include <fstream>

#include "subtitler/gui/auto_transcribe/auto_transcribe_window.h"
#include "subtitler/speech_recognition/auto_transcriber.h"
#include "subtitler/speech_recognition/cloud_service/microsoft_cognitive_service.h"
#include "subtitler/speech_recognition/languages/english_us.h"
#include "subtitler/srt/subrip_file.h"
#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/processing/ffmpeg.h"

namespace subtitler {
namespace gui {
namespace auto_transcribe {
namespace tasks {

TranscribeTask::TranscribeTask(QString api_key, QString api_region,
                               QString input_video, QString output_srt,
                               AutoTranscribeWindow* parent)
    : api_key_{api_key},
      api_region_{api_region},
      input_video_{input_video},
      output_srt_{output_srt},
      parent_{parent} {};

void TranscribeTask::run() {
  using namespace subtitler::speech_recognition;

  try {
    // Create a temp wav file.
    QDir parent_dir = QFileInfo{input_video_}.dir();
    QTemporaryFile temp(parent_dir.absoluteFilePath("XXXXXX.wav"));
    temp.open();

    // Extract Uncompressed audio.
    QMetaObject::invokeMethod(parent_, "onProgressUpdate",
                              Q_ARG(QString, "Extracting uncompressed audio"));
    std::string ffmpeg_path =
        QCoreApplication::applicationDirPath().toStdString() + "/ffmpeg";
    video::processing::FFMpeg ffmpeg{
        ffmpeg_path, std::make_unique<subprocess::SubprocessExecutor>()};
    ffmpeg.ExtractUncompressedAudio(input_video_.toStdString(),
                                    temp.fileName().toStdString());

    auto mcs_cloud_service =
        std::make_unique<cloud_service::MicrosoftCognitiveService>(
            api_key_.toStdString(), api_region_.toStdString());
    auto english_us = std::make_unique<languages::EnglishUS>();
    auto auto_transcriber = std::make_unique<AutoTranscriber>(
        std::move(mcs_cloud_service), std::move(english_us));

    // Upload audio for auto transcription.
    QMetaObject::invokeMethod(parent_, "onProgressUpdate",
                              Q_ARG(QString, "Uploading audio"));
    auto srt = auto_transcriber->Transcribe(
        temp.fileName().toStdString(), [this](const std::string& msg) {
          QMetaObject::invokeMethod(
              parent_, "onProgressUpdate",
              Q_ARG(QString, QString::fromStdString(msg)));
        });

    // Write results to srt file.
    QMetaObject::invokeMethod(parent_, "onProgressUpdate",
                              Q_ARG(QString, "Writing result"));
    std::ofstream output_file{
        std::filesystem::u8path(output_srt_.toStdString())};
    if (!output_file) {
      throw std::runtime_error{"Could not open output file for reading"};
    }
    srt.ToStream(output_file);

  } catch (const std::exception& e) {
    QMetaObject::invokeMethod(parent_, "onTranscribeComplete",
                              Q_ARG(QString, e.what()));
    return;
  } catch (...) {
    QMetaObject::invokeMethod(parent_, "onTranscribeComplete",
                              Q_ARG(QString, "unknown error"));
    return;
  }

  QMetaObject::invokeMethod(parent_, "onTranscribeComplete",
                            Q_ARG(QString, ""));
}

}  // namespace tasks
}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler
