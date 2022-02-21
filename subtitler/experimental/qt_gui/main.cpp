#include <QApplication>
#include <QDir>
#include <QFile>

#include "subtitler/experimental/qt_gui/player_window.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(timeline);

    QFile file(":/style/app");
    bool ret = file.open(QFile::ReadOnly);
    if (ret) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }

    PlayerWindow player_window;
    player_window.show();
    
    return app.exec();
}
