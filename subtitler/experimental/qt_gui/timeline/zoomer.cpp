#include "subtitler/experimental/qt_gui/timeline/zoomer.h"

#include <QHBoxLayout>
#include <iostream>
#include <cmath>

namespace {

int computeZoomRange(quint32 duration) {
    // The max interval size should be 10% of the total duration.
    double max_interval_size = duration / 10.0;
    // Each increase in the zoom level adds 10 secs to the interval.
    // Therefore, the range of the zoom levels is max_interval_size / 10.
    return std::max(1, static_cast<int>(std::ceil(max_interval_size / 10.0)));
}

} // namespace

Zoomer::Zoomer(QWidget* parent, quint32 duration)
    : QWidget(parent),
      zoom_in_{Q_NULLPTR},
      zoom_out_{Q_NULLPTR},
      zoom_slider_{Q_NULLPTR},
      current_level_{1},
      min_zoom_level_{1},
      max_zoom_level_{1} {
    setFixedWidth(150);
    setCursor(Qt::PointingHandCursor);
    InitializeControls(duration);
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(zoom_in_);
    mainLayout->addWidget(zoom_slider_);
    mainLayout->addWidget(zoom_out_);
    setLayout(mainLayout);
}

void Zoomer::InitializeControls(quint32 duration) {
    zoom_slider_ = new QSlider(Qt::Horizontal, this);
    // Double the width
    min_zoom_level_ = 1;
    max_zoom_level_ = computeZoomRange(duration);

    zoom_slider_->setRange(min_zoom_level_, max_zoom_level_);
    zoom_slider_->setSliderPosition(max_zoom_level_);

    zoom_in_ = new QToolButton(this);
    zoom_in_->setIcon(QIcon(":/images/zoomin"));
    zoom_out_ = new QToolButton(this);
    zoom_out_->setIcon(QIcon(":/images/zoomout"));

    connect(zoom_slider_, &QAbstractSlider::valueChanged, this,
            &Zoomer::onSliderChanged);
    connect(zoom_in_, &QAbstractButton::clicked, this, &Zoomer::onZoomInClicked);
    connect(zoom_out_, &QAbstractButton::clicked, this,
            &Zoomer::onZoomOutClicked);
}

void Zoomer::onZoomInClicked(bool checked) {
    int currentValue = zoom_slider_->value();
    if (currentValue > min_zoom_level_) {
        zoom_slider_->setSliderPosition(--currentValue);
        emit zoomerIn(currentValue);
    }
}

void Zoomer::onZoomOutClicked(bool checked) {
    int currentValue = zoom_slider_->value();
    if (currentValue < max_zoom_level_) {
        zoom_slider_->setSliderPosition(++currentValue);
        emit zoomerOut(currentValue);
    }
}

void Zoomer::onSliderChanged(int value) {
    if (value < min_zoom_level_ || value > max_zoom_level_) {
        return;
    }
    if (value > current_level_) {
        emit zoomerOut(value);
    } else if (value < current_level_) {
        emit zoomerIn(value);
    }
    current_level_ = value;
    zoom_slider_->setSliderPosition(current_level_);
}

void Zoomer::onChangeSliderPosition(int level) {
    onSliderChanged(level);
}
