#include "subtitler/gui/timeline/timer.h"

#include "subtitler/util/duration_format.h"

using namespace std::chrono_literals;

Timer::Timer(QWidget* parent) : QLabel{parent} {
    setText(QString::fromStdString(subtitler::FormatDuration(0ms)));
}

void Timer::onTimerChanged(std::chrono::milliseconds ms) {
    setText(QString::fromStdString(subtitler::FormatDuration(ms)));
}
