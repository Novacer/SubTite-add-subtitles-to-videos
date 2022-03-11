#include "subtitler/gui/timeline/timeline.h"

#include <QDebug>
#include <QScrollBar>
#include <QVBoxLayout>
#include <chrono>

using namespace std::chrono_literals;

Timeline::Timeline(std::chrono::milliseconds duration, QWidget* parent)
    : QScrollArea(parent) {
    setMinimumSize(1000, 150);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
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
    connect(ruler_, &Ruler::subtitleIntervalClicked, this,
            &Timeline::onSubtitleIntervalClicked);
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

void Timeline::onPlayerPause() { ruler_->setPlaying(false); }

void Timeline::onPlayerPlay() { ruler_->setPlaying(true); }

void Timeline::onSubtitleIntervalClicked(SubtitleInterval* subtitle) {
    emit openSubtitleEditor(subtitle);
}
