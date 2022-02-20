#include "subtitler/experimental/qt_gui/timeline/keypoint.h"

#include <QVariant>

KeyPoint::KeyPoint(QWidget* parent /* = Q_NULLPTR */) : QWidget(parent) {
    setProperty("type", "point");
    setAttribute(Qt::WA_TranslucentBackground, true);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(8, 8);
    show();
}

void KeyPoint::paintEvent(QPaintEvent* event) {}
