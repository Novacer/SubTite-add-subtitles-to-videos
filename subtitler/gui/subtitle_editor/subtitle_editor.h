#ifndef SUBTITLER_GUI_SUBTITLE_EDITOR_SUBTITLE_EDITOR_H
#define SUBTITLER_GUI_SUBTITLE_EDITOR_SUBTITLE_EDITOR_H

#include <QDockWidget>
#include <string>

QT_FORWARD_DECLARE_CLASS(QPlainTextEdit)
QT_FORWARD_DECLARE_CLASS(QLabel)

namespace subtitler::gui {
namespace timeline {

QT_FORWARD_DECLARE_CLASS(SubtitleInterval)
QT_FORWARD_DECLARE_CLASS(SubtitleIntervalContainer)

}  // namespace timeline

namespace subtitle_editor {

QT_FORWARD_DECLARE_CLASS(PositionButtons)

}  // namespace subtitle_editor

}  // namespace subtitler::gui

namespace subtitler {
namespace gui {
namespace subtitle_editor {

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
  void onOpenSubtitle(timeline::SubtitleIntervalContainer* container,
                      timeline::SubtitleInterval* subtitle);
  void onSubtitleTextChanged();
  void onSubtitleChangeStartEndTime(timeline::SubtitleInterval* subtitle);

  // Pass a container pointer to use this new container for successive calls.
  // Pass null container to keep using the last container.
  void onSave(timeline::SubtitleIntervalContainer* container = Q_NULLPTR);
  void onDelete();
  void onVisibilityChanged(bool visible);
  void onPositionSelected(const std::string& position_id);
  void onSubtitleFileReload(const QString& new_subtitle_file);

 protected:
  virtual bool eventFilter(QObject* watched, QEvent* event) override;

 private:
  QLabel* begin_end_time_;
  QPlainTextEdit* text_edit_;
  PositionButtons* position_buttons_;

  timeline::SubtitleInterval* currently_editing_;
  timeline::SubtitleIntervalContainer* container_;
  bool prev_visibility_;
};

}  // namespace subtitle_editor
}  // namespace gui
}  // namespace subtitler

#endif
