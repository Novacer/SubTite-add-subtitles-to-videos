#include "subtitler/gui/timeline/indicator.h"

namespace subtitler {
namespace gui {
namespace timeline {

Indicator::Indicator(QWidget* parent /* = Q_NULLPTR */) : QLabel(parent) {
  setAttribute(Qt::WA_TranslucentBackground, true);
  setCursor(Qt::SizeHorCursor);
  setPixmap(QPixmap(":/images/indicator"));
  setScaledContents(true);
  setMinimumSize(19, 200);
  move(0, 40);
}

}  // namespace timeline
}  // namespace gui
}  // namespace subtitler
