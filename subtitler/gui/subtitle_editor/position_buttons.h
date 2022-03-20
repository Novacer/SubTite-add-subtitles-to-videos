#ifndef SUBTITLER_GUI_SUBTITLE_EDITOR_POSITION_BUTTONS_H
#define SUBTITLER_GUI_SUBTITLE_EDITOR_POSITION_BUTTONS_H

#include <QWidget>
#include <string>

QT_FORWARD_DECLARE_CLASS(QRadioButton)

namespace subtitler {
namespace gui {
namespace subtitle_editor {

/**
 * A 3x3 grid of radio buttons which are used to select the position where
 * something (ex. subtitles) will show up on the edited video.
 */
class PositionButtons : public QWidget {
    Q_OBJECT
  public:
    PositionButtons(QWidget* parent = Q_NULLPTR);
    ~PositionButtons() = default;

  signals:
    /**
     * The possible position_ids are defined as the keys of the map in
     * subtitler/srt/subrip_item.h
     */
    void positionIdSelected(const std::string& position_id);

  public slots:
    /**
     * The substation_alpha_ids are defined as the values of the map in
     * subtitler/srt/subrip_item.h
     * Providing an invalid key with throw std::invalid_argument.
     */
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

}  // namespace subtitle_editor
}  // namespace gui
}  // namespace subtitler

#endif
