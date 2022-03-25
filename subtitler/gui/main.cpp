#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavutil/log.h>
}

#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <memory>
#include <iostream>

#include "subtitler/gui/main_window.h"
#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/processing/ffmpeg.h"

int main(int argc, char *argv[]) {
    av_log_set_level(AV_LOG_FATAL);
    QApplication app(argc, argv);

    QString app_path = QCoreApplication::applicationDirPath();

    auto executor =
        std::make_unique<subtitler::subprocess::SubprocessExecutor>();
    subtitler::video::processing::FFMpeg ffmpeg{
        app_path.toStdString() + "/ffmpeg", std::move(executor)};
    
    qDebug() << QString::fromStdString(ffmpeg.GetVersionInfo());

    app.setWindowIcon(QIcon(":/images/logo"));

    // Force software decoding for now, until I can figure out
    // how to make hw decoding work...
    qputenv("QT_AVPLAYER_NO_HWDEVICE", QByteArray("1"));

    Q_INIT_RESOURCE(timeline);

    QFile file(":/style/app");
    bool ret = file.open(QFile::ReadOnly);
    if (ret) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }

    int res = 0;

    try {
        subtitler::gui::MainWindow main_window;
        main_window.show();
        res = app.exec();
    } catch (const std::exception &e) {
        // Silence
    }

    qunsetenv("QT_AVPLAYER_NO_HWDEVICE");
    return res;
}
