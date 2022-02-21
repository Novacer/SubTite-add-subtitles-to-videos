#include "subtitler/experimental/qt_gui/timeline/timeline.h"

#include <QDebug>
#include <QScrollBar>
#include <QVBoxLayout>
#include <chrono>

using namespace std::chrono_literals;

Timeline::Timeline(QWidget* parent /* = Q_NULLPTR */) : QScrollArea(parent) {
    setMinimumSize(1000, 150);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    std::chrono::milliseconds duration = 10min;
    zoomer_ = new Zoomer(this, duration);
    zoomer_->setMinimumWidth(300);
    addScrollBarWidget(zoomer_, Qt::AlignLeft);

    ruler_ = new Ruler(this, duration, zoomer_->max_zoom_level());
    setWidget(ruler_);

    connect(zoomer_, &Zoomer::zoomIn, ruler_, &Ruler::onZoomIn);
    connect(zoomer_, &Zoomer::zoomOut, ruler_, &Ruler::onZoomOut);
    connect(ruler_, &Ruler::changeZoomPosition, zoomer_,
            &Zoomer::onSliderChanged);
    connect(ruler_, &Ruler::changeIndicatorTime, this,
            &Timeline::onRulerChangedTime);
    connect(this, &Timeline::playerChangedTime, ruler_,
            &Ruler::onMoveIndicator);
    connect(ruler_, &Ruler::userChangedIndicatorTime, this,
            &Timeline::onUserDraggedRulerChangeTime);
}

void Timeline::onRulerChangedTime(std::chrono::milliseconds ms) {
    emit rulerChangedTime(ms);
}

void Timeline::onPlayerChangedTime(std::chrono::milliseconds ms) {
    emit playerChangedTime(ms);
}

void Timeline::onUserDraggedRulerChangeTime(std::chrono::milliseconds ms) {
    emit userDraggedRulerChangeTime(ms);
}

void Timeline::onPlayerPause() {
    ruler_->setPlaying(false);
}

void Timeline::onPlayerPlay() {
    ruler_->setPlaying(true);
}
