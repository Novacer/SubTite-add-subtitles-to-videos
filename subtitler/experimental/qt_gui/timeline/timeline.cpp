#include "subtitler/experimental/qt_gui/timeline/timeline.h"

#include <QDebug>
#include <QScrollBar>
#include <QVBoxLayout>

Timeline::Timeline(QWidget* parent /* = Q_NULLPTR */)
    : QScrollArea(parent) {
    setMinimumSize(1000, 150);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    quint32 duration = 600;
    zoomer_ = new Zoomer(this, duration);
    zoomer_->setMinimumWidth(300);
    addScrollBarWidget(zoomer_, Qt::AlignLeft);

    ruler_ = new Ruler(this, duration, zoomer_->max_zoom_level());
    setWidget(ruler_);

    connect(zoomer_, &Zoomer::zoomIn, ruler_, &Ruler::onZoomIn);
    connect(zoomer_, &Zoomer::zoomOut, ruler_, &Ruler::onZoomOut);
    connect(ruler_, &Ruler::changeZoomPosition, zoomer_,
            &Zoomer::onSliderChanged);
    connect(ruler_, &Ruler::changeIndicatorTime, this, &Timeline::onChangeTime);
}

void Timeline::onChangeTime(std::chrono::milliseconds ms) {
    emit changeTime(ms);
}
