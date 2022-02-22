#include <QApplication>
#include <QDir>
#include <QFile>
#include <iostream>

#include "subtitler/experimental/qt_gui/timeline/mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    Q_INIT_RESOURCE(timeline);

    QFile file(":/style/app");
    bool ret = file.open(QFile::ReadOnly);
    if (ret) {
        qApp->setStyleSheet(file.readAll());
        file.close();
    }

    MainWindow timeline;
    timeline.show();

    return app.exec();
}
