#ifndef SUBTITLER_INDICATOR_H
#define SUBTITLER_INDICATOR_H

#include <QLabel>

class Indicator : public QLabel {
    Q_OBJECT
  public:
    Indicator(QWidget* parent = Q_NULLPTR);
    ~Indicator() = default;
};

#endif
