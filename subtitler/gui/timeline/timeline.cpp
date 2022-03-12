#include "subtitler/gui/timeline/timeline.h"

#include <QDebug>
#include <QScrollBar>
#include <QVBoxLayout>
#include <chrono>

using namespace std::chrono_literals;

namespace subtitler {
namespace gui {

Timeline::Timeline(std::chrono::milliseconds duration,
                   const QString& output_srt_file, QWidget* parent)
    : QScrollArea(parent) {
    setMinimumSize(1280, 150);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    zoomer_ = new Zoomer(this, duration);
    zoomer_->setMinimumWidth(300);
    addScrollBarWidget(zoomer_, Qt::AlignLeft);

    ruler_ =
        new Ruler(this, duration, output_srt_file, zoomer_->GetMaxZoomLevel());
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
    connect(ruler_, &Ruler::changeSubtitleIntervalTime, this,
            &Timeline::onChangeSubtitleStartEndTime);
    connect(ruler_, &Ruler::changeSubtitleIntervalTimeFinished, this,
            &Timeline::onChangeSubtitleStartEndTimeFinished);
    connect(ruler_, &Ruler::subtitleFileLoaded, this,
            &Timeline::onSubtitleFileLoaded);
}

void Timeline::LoadSubtitles() { ruler_->LoadSubtitles(); }

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

void Timeline::onSubtitleIntervalClicked(SubtitleIntervalContainer* container,
                                         SubtitleInterval* subtitle) {
    emit openSubtitleEditor(container, subtitle);
}

void Timeline::onChangeSubtitleStartEndTime(SubtitleInterval* subtitle) {
    emit changeSubtitleStartEndTime(subtitle);
}

void Timeline::onChangeSubtitleStartEndTimeFinished(
    SubtitleInterval* subtitle) {
    emit changeSubtitleStartEndTimeFinished(subtitle);
}

void Timeline::onSubtitleFileLoaded() { emit subtitleFileLoaded(); }

}  // namespace gui
}  // namespace subtitler
