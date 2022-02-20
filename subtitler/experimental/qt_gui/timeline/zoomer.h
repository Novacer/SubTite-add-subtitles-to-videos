#ifndef SUBTITLER_ZOOMER_H
#define SUBTITLER_ZOOMER_H

#include <QSlider>
#include <QToolButton>
#include <QWidget>

class Zoomer : public QWidget {
    Q_OBJECT
  public:
    Zoomer(QWidget* parent = Q_NULLPTR);
    void initializeControls();
    ~Zoomer() = default;

  signals:
    void zoomerIn(int level);
    void zoomerOut(int level);

  public slots:
    void onZoomInClicked(bool checked);
    void onZoomOutClicked(bool checked);
    void onSliderChanged(int value);
    void onChangeSliderPosition(int level);

  private:
    QToolButton* mZoomIn;
    QToolButton* mZoomOut;
    QSlider* mZoomSlider;
    int mCurrentLevel;
};

#endif
