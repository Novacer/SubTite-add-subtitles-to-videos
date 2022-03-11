#ifndef SUBTITLER_TIMER_H
#define SUBTITLER_TIMER_H

#include <QLabel>
#include <chrono>

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

#endif
