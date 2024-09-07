#ifndef SUBTITLER_GUI_TIMELINE_ZOOMER_H
#define SUBTITLER_GUI_TIMELINE_ZOOMER_H

#include <QSlider>
#include <QToolButton>
#include <QWidget>
#include <chrono>

namespace subtitler {
namespace gui {
namespace timeline {

/**
 * Scrollbar widget to control the timeline's zoom level.
 */
class Zoomer : public QWidget {
    Q_OBJECT
  public:
    Zoomer(QWidget* parent, std::chrono::milliseconds duration);
    ~Zoomer() = default;

    int GetCurrentZoomLevel() const { return current_level_; }

  signals:
    void zoomIn(int level);
    void zoomOut(int level);

  public slots:
    void onZoomInClicked(bool checked);
    void onZoomOutClicked(bool checked);
    void onSliderChanged(int value);

  private:
    void initializeControls(std::chrono::milliseconds duration);

    QToolButton* zoom_in_;
    QToolButton* zoom_out_;
    QSlider* zoom_slider_;
    int current_level_;
    int min_zoom_level_;
    int max_zoom_level_;
};

}  // namespace timeline
}  // namespace gui
}  // namespace subtitler

#endif
