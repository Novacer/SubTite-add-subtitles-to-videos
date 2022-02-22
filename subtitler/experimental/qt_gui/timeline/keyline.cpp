#include "subtitler/experimental/qt_gui/timeline/keyline.h"

#include <QDebug>
#include <QPaintEvent>

KeyLine::KeyLine(QWidget* parent /* = Q_NULLPTR */) : QFrame(parent) {}

void KeyLine::paintEvent(QPaintEvent* event) {
    qDebug() << "KeyLine Paint Event region height:"
             << event->region().boundingRect().height();
}
