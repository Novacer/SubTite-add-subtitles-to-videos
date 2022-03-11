#ifndef SUBTITLER_GUI_INDICATOR_H
#define SUBTITLER_GUI_INDICATOR_H

#include <QLabel>

namespace subtitler {
namespace gui {

class Indicator : public QLabel {
    Q_OBJECT
  public:
    Indicator(QWidget* parent = Q_NULLPTR);
    ~Indicator() = default;
};

}  // namespace gui
}  // namespace subtitler

#endif
