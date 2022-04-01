#include "subtitler/gui/exporting/tasks/burn_subtitle_task.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMetaObject>
#include <QMetaType>

#include "subtitler/gui/exporting/export_dialog.h"
#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/processing/ffmpeg.h"

// Reference https://stackoverflow.com/a/14713932/17786559
// Both Q_DECLARE_METATYPE and qRegisterMetaType in the ctor are needed
// so that QT can pass Progress through QMetaObject::invokeMethod
Q_DECLARE_METATYPE(subtitler::video::processing::Progress)

namespace subtitler {
namespace gui {
namespace exporting {
namespace tasks {

BurnSubtitleTask::BurnSubtitleTask(QString video, QString subtitle,
                                   QString output, ExportWindow *parent)
    : QRunnable{},
      video_{video},
      subtitle_{subtitle},
      output_{output},
      parent_{parent} {
    qRegisterMetaType<subtitler::video::processing::Progress>();
}

void BurnSubtitleTask::run() {
    std::string ffmpeg_path =
        QCoreApplication::applicationDirPath().toStdString() + "/ffmpeg";

    video::processing::FFMpeg ffmpeg{
        ffmpeg_path, std::make_unique<subprocess::SubprocessExecutor>()};

    try {
        ffmpeg.BurnSubtitlesAsync(
            video_.toStdString(), subtitle_.toStdString(),
            output_.toStdString(),
            [this](const video::processing::Progress &progress) {
                QMetaObject::invokeMethod(
                    parent_, "onProgressUpdate",
                    // Q_ARG expects fully qualified name.
                    Q_ARG(subtitler::video::processing::Progress, progress));
            });
        ffmpeg.WaitForAsyncTask();
        QMetaObject::invokeMethod(parent_, "onExportComplete",
                                  Q_ARG(QString, ""));
    } catch (const std::exception &e) {
        qDebug() << "Error starting ffmpeg: " << e.what();
        QMetaObject::invokeMethod(parent_, "onExportComplete",
                                  Q_ARG(QString, e.what()));
    }
}

}  // namespace tasks
}  // namespace exporting
}  // namespace gui
}  // namespace subtitler
