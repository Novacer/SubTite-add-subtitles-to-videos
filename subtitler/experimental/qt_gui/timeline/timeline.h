#ifndef SUBTITLER_TIMELINE_H
#define SUBTITLER_TIMELINE_H

#include <QScrollArea>
#include <chrono>

#include "subtitler/experimental/qt_gui/timeline/ruler.h"
#include "subtitler/experimental/qt_gui/timeline/zoomer.h"

class Timeline : public QScrollArea {
    Q_OBJECT
  public:
    Timeline(QWidget* parent = Q_NULLPTR);
    ~Timeline() = default;

  signals:
    void changeTime(std::chrono::milliseconds ms);
  
  public slots:
    void onChangeTime(std::chrono::milliseconds ms);

  private:
    Ruler* ruler_;
    Zoomer* zoomer_;
};

#endif
