#ifndef SUBTITLER_TIMELINE_H
#define SUBTITLER_TIMELINE_H

#include <QScrollArea>
#include <chrono>

#include "subtitler/experimental/qt_gui/timeline/ruler.h"
#include "subtitler/experimental/qt_gui/timeline/zoomer.h"

QT_FORWARD_DECLARE_CLASS(SubtitleInterval);

class Timeline : public QScrollArea {
    Q_OBJECT
  public:
    Timeline(std::chrono::milliseconds duration, QWidget* parent = Q_NULLPTR);
    ~Timeline() = default;

  signals:
    void rulerChangedTime(std::chrono::milliseconds ms);
    void playerChangedTime(std::chrono::milliseconds ms);
    void userDraggedRulerChangeTime(std::chrono::milliseconds ms);
    void openSubtitleEditor(SubtitleInterval* subtitle);
  
  public slots:
    // Handles outgoing time changes from the ruler.
    void onRulerChangedTime(std::chrono::milliseconds ms);
    // Handles outgoing time changes of the ruler
    // caused by the user dragging the indicator with their mouse.
    void onUserDraggedRulerChangeTime(std::chrono::milliseconds ms);
    // Handles incoming time changes from the player.
    void onPlayerChangedTime(std::chrono::milliseconds ms);
    
    void onPlayerPause();
    void onPlayerPlay();

    void onSubtitleIntervalClicked(SubtitleInterval* subtitle);

  private:
    Ruler* ruler_;
    Zoomer* zoomer_;
};

#endif
