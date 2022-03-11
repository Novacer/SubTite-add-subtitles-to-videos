#ifndef SUBTITLER_GUI_TIMELINE_H
#define SUBTITLER_GUI_TIMELINE_H

#include <QScrollArea>
#include <chrono>

#include "subtitler/gui/timeline/ruler.h"
#include "subtitler/gui/timeline/zoomer.h"

namespace subtitler::gui {

QT_FORWARD_DECLARE_CLASS(SubtitleInterval);

}  // namespace subtitler::gui

namespace subtitler {
namespace gui {

/**
 * Widget containing the scrollable timeline controls below the video player.
 * Includes the zoomable video timeline, current position indicator,
 * and subtitle positions.
 */
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

}  // namespace gui
}  // namespace subtitler

#endif
