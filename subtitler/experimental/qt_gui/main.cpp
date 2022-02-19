// Example app initially taken from
// https://github.com/vivaladav/BitsOfBytes/tree/master/how-to-create-an-application-with-qt-and-cpp
// TODO: replace this with a demo video player eventually

#include <QApplication>

#include "subtitler/experimental/qt_gui/main_window.h"

int main(int argc, char **argv) {
    QApplication app{argc, argv};

    MainWindow window;
    window.show();

    return app.exec();
}
