#ifndef SUBTITLER_GUI_MAIN_WINDOW_H
#define SUBTITLER_GUI_MAIN_WINDOW_H

#include <QMainWindow>
#include <QString>
#include <chrono>
#include <memory>
#include <optional>

#include "subtitler/gui/auto_transcribe/auto_transcribe_window.h"
#include "subtitler/gui/auto_transcribe/login/login_msc.h"
#include "subtitler/gui/subtitle_editor/subtitle_editor.h"
#include "subtitler/gui/video_renderer/opengl_renderer.h"

QT_FORWARD_DECLARE_CLASS(QAVPlayer)
QT_FORWARD_DECLARE_CLASS(QAVAudioOutput)
QT_FORWARD_DECLARE_CLASS(QAVAudioFrame)
QT_FORWARD_DECLARE_CLASS(QAVVideoFrame)
QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(QDialog)

namespace subtitler {
namespace gui {

/**
 * Main window for the video editor gui.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  MainWindow(QWidget* parent = Q_NULLPTR);
  ~MainWindow();

 signals:
  void playerChangedTime(std::chrono::milliseconds ms);
  void subtitleFileReload(const QString& new_subtitle_file);

 public slots:
  void onRulerChangedTime(std::chrono::milliseconds ms);
  void onAudioFrameDecoded(const QAVAudioFrame& audio_frame);
  void onVideoFrameDecoded(const QAVVideoFrame& video_frame);
  void onSubtitleFileChanged(std::size_t num_loaded);
  void onExport(bool checked);
  void onAutoTranscribe(bool checked);
  void onSubtitleFileReload(const QString& new_subtitle_file);

 private:
  std::unique_ptr<QAVPlayer> player_;
  std::unique_ptr<QAVAudioOutput> audio_output_;
  std::unique_ptr<QFile> video_file_;
  video_renderer::OpenGLRenderer* video_renderer_;
  subtitle_editor::SubtitleEditor* editor_;
  // Flag var which is set when user moves the ruler,
  // and reset once the next video frame is decoded.
  bool user_seeked_;
  QString subtitle_file_;
  QDialog* export_dialog_;
  auto_transcribe::AutoTranscribeWindow* auto_transcribe_window_;
  auto_transcribe::login::LoginMicrosoftCognitiveServicesWindow* login_window_;

  // Initially nullopt. Value will be set after the first frame is decoded.
  // Note that the filter value may be empty string, if no downscaling is
  // needed.
  std::optional<QString> downscale_filter_;
};

}  // namespace gui
}  // namespace subtitler

#endif
