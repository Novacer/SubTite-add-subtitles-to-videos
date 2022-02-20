#ifndef SUBTITLER_KEY_LINE_H
#define SUBTITLER_KEY_LINE_H

#include <QFrame>

class KeyLine : public QFrame {
    Q_OBJECT
  public:
    KeyLine(QWidget* parent = Q_NULLPTR);
    ~KeyLine() = default;

  protected:
    virtual void paintEvent(QPaintEvent* event) override;
};

#endif
