#ifndef SUBTITLER_GUI_TIMELINE_INDICATOR_H
#define SUBTITLER_GUI_TIMELINE_INDICATOR_H

#include <QLabel>

namespace subtitler {
namespace gui {
namespace timeline {

/**
 * Vertical bar showing the current position of the video player.
 */
class Indicator : public QLabel {
    Q_OBJECT
  public:
    Indicator(QWidget* parent = Q_NULLPTR);
    ~Indicator() = default;
};

}  // namespace timeline
}  // namespace gui
}  // namespace subtitler

#endif
