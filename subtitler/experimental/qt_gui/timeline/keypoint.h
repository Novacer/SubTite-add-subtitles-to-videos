#ifndef SUBTITLER_KEY_POINT_H
#define SUBTITLER_KEY_POINT_H

#include <QWidget>

class KeyPoint : public QWidget {
    Q_OBJECT
  public:
    KeyPoint(QWidget* parent = Q_NULLPTR);
    ~KeyPoint() = default;

  protected:
    virtual void paintEvent(QPaintEvent* event) override;
};

#endif
