#ifndef SUBTITLER_ZOOMER_H
#define SUBTITLER_ZOOMER_H

#include <QSlider>
#include <QToolButton>
#include <QWidget>
#include <chrono>

class Zoomer : public QWidget {
    Q_OBJECT
  public:
    Zoomer(QWidget* parent, std::chrono::milliseconds duration);
    ~Zoomer() = default;

    void InitializeControls(std::chrono::milliseconds duration);

    int max_zoom_level() const { return max_zoom_level_; };

  signals:
    void zoomIn(int level);
    void zoomOut(int level);

  public slots:
    void onZoomInClicked(bool checked);
    void onZoomOutClicked(bool checked);
    void onSliderChanged(int value);

  private:
    QToolButton* zoom_in_;
    QToolButton* zoom_out_;
    QSlider* zoom_slider_;
    int current_level_;
    int min_zoom_level_;
    int max_zoom_level_;
};

#endif
