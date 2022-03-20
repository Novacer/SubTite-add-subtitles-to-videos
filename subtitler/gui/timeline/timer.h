#ifndef SUBTITLER_GUI_TIMELINE_TIMER_H
#define SUBTITLER_GUI_TIMELINE_TIMER_H

#include <QLabel>
#include <chrono>

namespace subtitler {
namespace gui {
namespace timeline {

/**
 * Label displaying a timestamp in HH:MM:SS.XXXXX format.
 */
class Timer : public QLabel {
    Q_OBJECT
  public:
    Timer(QWidget* parent = Q_NULLPTR);
    ~Timer() = default;

  public slots:
    // Set the new time.
    void onTimerChanged(std::chrono::milliseconds ms);
};

}  // namespace timeline
}  // namespace gui
}  // namespace subtitler

#endif
