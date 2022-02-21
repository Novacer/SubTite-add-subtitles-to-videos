#ifndef SUBTITLER_MAIN_WINDOW_H
#define SUBTITLER_MAIN_WINDOW_H

#include <QScrollArea>

#include "subtitler/experimental/qt_gui/timeline/ruler.h"
#include "subtitler/experimental/qt_gui/timeline/zoomer.h"

class MainWindow : public QScrollArea {
    Q_OBJECT
  public:
    MainWindow(QWidget* parent = Q_NULLPTR);
    ~MainWindow() = default;

  private:
    Ruler* ruler_;
    Zoomer* zoomer_;
};

#endif
