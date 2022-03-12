#ifndef SUBTITLER_GUI_SUBTITLE_EDITOR_H
#define SUBTITLER_GUI_SUBTITLE_EDITOR_H

#include <QDockWidget>

QT_FORWARD_DECLARE_CLASS(QPlainTextEdit)
QT_FORWARD_DECLARE_CLASS(QLabel)

namespace subtitler::gui {

QT_FORWARD_DECLARE_CLASS(SubtitleInterval)

}  // namespace subtitler::gui

namespace subtitler {
namespace gui {

/**
 * Side docking widget containing elements used when editing
 * the subtitle text.
 */
class SubtitleEditor : public QDockWidget {
    Q_OBJECT
  public:
    SubtitleEditor(QWidget* parent = Q_NULLPTR);
    ~SubtitleEditor() = default;

  public slots:
    void onOpenSubtitle(SubtitleInterval* subtitle);
    void onSubtitleTextChanged();
    void onSubtitleChangeStartEndTime(SubtitleInterval* subtitle);

  private:
    QPlainTextEdit* text_edit_;
    QLabel* begin_end_time_;
    SubtitleInterval* currently_editing_;
};

}  // namespace gui
}  // namespace subtitler

#endif
