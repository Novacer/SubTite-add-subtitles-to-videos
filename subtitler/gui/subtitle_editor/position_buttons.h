#ifndef SUBTITLER_GUI_SUBTITLE_EDITOR_POSITION_BUTTONS_H
#define SUBTITLER_GUI_SUBTITLE_EDITOR_POSITION_BUTTONS_H

#include <QWidget>
#include <string>

QT_FORWARD_DECLARE_CLASS(QRadioButton)

namespace subtitler {
namespace gui {

class PositionButtons : public QWidget {
    Q_OBJECT
  public:
    PositionButtons(QWidget* parent = Q_NULLPTR);
    ~PositionButtons() = default;

  signals:
    void positionIdSelected(const std::string& position_id);

  public slots:
    void onPositionChanged(int substation_alpha_id);

  private:
    QRadioButton* radio_top_left_;
    QRadioButton* radio_top_center_;
    QRadioButton* radio_top_right_;
    QRadioButton* radio_middle_left_;
    QRadioButton* radio_middle_center_;
    QRadioButton* radio_middle_right_;
    QRadioButton* radio_bottom_left_;
    QRadioButton* radio_bottom_center_;
    QRadioButton* radio_bottom_right_;
};

}  // namespace gui
}  // namespace subtitler

#endif
