#ifndef SUBTITLER_SUBTITLE_EDITOR
#define SUBTITLER_SUBTITLE_EDITOR

#include <QDockWidget>

QT_FORWARD_DECLARE_CLASS(QPlainTextEdit)
QT_FORWARD_DECLARE_CLASS(SubtitleInterval)

class SubtitleEditor : public QDockWidget {
    Q_OBJECT
  public:
    SubtitleEditor(QWidget* parent = Q_NULLPTR);
    ~SubtitleEditor() = default;

  public slots:
    void onOpenSubtitle(SubtitleInterval* subtitle);
    void onSubtitleTextChanged();

  private:
    QPlainTextEdit* text_edit_;
    SubtitleInterval* currently_editing_;
};

#endif
