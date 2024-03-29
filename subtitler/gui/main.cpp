#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavutil/log.h>
}

#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>

#include "subtitler/gui/main_window.h"

int main(int argc, char *argv[]) {
    av_log_set_level(AV_LOG_FATAL);
    QApplication app(argc, argv);
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
