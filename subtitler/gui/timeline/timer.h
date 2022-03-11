#ifndef SUBTITLER_GUI_TIMER_H
#define SUBTITLER_GUI_TIMER_H

#include <QLabel>
#include <chrono>

namespace subtitler {
namespace gui {

class Timer : public QLabel {
    Q_OBJECT
  public:
    Timer(QWidget* parent = Q_NULLPTR);
    ~Timer() = default;

  signals:
    void timerChanged(std::chrono::milliseconds ms);

  public slots:
    void onTimerChanged(std::chrono::milliseconds ms);
};

}  // namespace gui
}  // namespace subtitler

#endif
