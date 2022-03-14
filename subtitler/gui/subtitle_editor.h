#ifndef SUBTITLER_GUI_SUBTITLE_EDITOR_H
#define SUBTITLER_GUI_SUBTITLE_EDITOR_H

#include <QDockWidget>

QT_FORWARD_DECLARE_CLASS(QPlainTextEdit)
QT_FORWARD_DECLARE_CLASS(QLabel)

namespace subtitler::gui {

QT_FORWARD_DECLARE_CLASS(SubtitleInterval)
QT_FORWARD_DECLARE_CLASS(SubtitleIntervalContainer)

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

    std::size_t GetNumSubtitles() const;

  signals:
    void saved(std::size_t num_subtitles);

  public slots:
    void onOpenSubtitle(SubtitleIntervalContainer* container,
                        SubtitleInterval* subtitle);
    void onSubtitleTextChanged();
    void onSubtitleChangeStartEndTime(SubtitleInterval* subtitle);
    void onSave();
    void onDelete();
    void onVisibilityChanged(bool visible);

  protected:
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

  private:
    QPlainTextEdit* text_edit_;
    QLabel* begin_end_time_;
    SubtitleInterval* currently_editing_;
    SubtitleIntervalContainer* container_;
    bool prev_visibility_;
};

}  // namespace gui
}  // namespace subtitler

#endif
