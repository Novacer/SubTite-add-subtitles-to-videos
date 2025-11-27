#ifndef SUBTITLER_GUI_TIMELINE_TIMELINE_H
#define SUBTITLER_GUI_TIMELINE_TIMELINE_H

#include <QScrollArea>
#include <chrono>

#include "subtitler/gui/timeline/ruler.h"
#include "subtitler/gui/timeline/zoomer.h"

namespace subtitler {
namespace gui {
namespace timeline {

QT_FORWARD_DECLARE_CLASS(SubtitleInterval);
QT_FORWARD_DECLARE_CLASS(SubtitleIntervalContainer);

/**
 * Widget containing the scrollable timeline controls below the video player.
 * Includes the zoomable video timeline, current position indicator,
 * and subtitle positions.
 */
class Timeline : public QScrollArea {
  Q_OBJECT
 public:
  Timeline(std::chrono::milliseconds duration, const QString& output_srt_file,
           QWidget* parent = Q_NULLPTR);
  ~Timeline() = default;

  // Loads internal state from the external output_srt_file.
  void LoadSubtitles();

 signals:
  void rulerChangedTime(std::chrono::milliseconds ms);
  void playerChangedTime(std::chrono::milliseconds ms);
  void userDraggedRulerChangeTime(std::chrono::milliseconds ms);
  void openSubtitleEditor(SubtitleIntervalContainer* container,
                          SubtitleInterval* subtitle);
  void changeSubtitleStartEndTime(SubtitleInterval* subtitle);
  void changeSubtitleStartEndTimeFinished(SubtitleIntervalContainer* container,
                                          SubtitleInterval* subtitle);
  void subtitleFileLoaded(std::size_t num_loaded);

 public slots:
  // Handles outgoing time changes from the ruler.
  void onRulerChangedTime(std::chrono::milliseconds ms);
  // Handles outgoing time changes of the ruler
  // caused by the user dragging the indicator with their mouse.
  void onUserDraggedRulerChangeTime(std::chrono::milliseconds ms);
  // Handles incoming time changes from the player.
  void onPlayerChangedTime(std::chrono::milliseconds ms);
  // Handles incoming time changes when user steps forwards/backwards.
  void onUserStepChangedTime(std::chrono::milliseconds delta);

  void onPlayerPause();
  void onPlayerPlay();

  void onSubtitleIntervalClicked(SubtitleIntervalContainer* container,
                                 SubtitleInterval* subtitle);
  void onChangeSubtitleStartEndTime(SubtitleInterval* subtitle);
  void onChangeSubtitleStartEndTimeFinished(
      SubtitleIntervalContainer* container, SubtitleInterval* subtitle);
  void onSubtitleFileLoaded(std::size_t num_loaded);
  void onSubtitleFileReload(const QString& new_subtitle_file);

 private:
  Ruler* ruler_;
  Zoomer* zoomer_;
};

}  // namespace timeline
}  // namespace gui
}  // namespace subtitler

#endif
