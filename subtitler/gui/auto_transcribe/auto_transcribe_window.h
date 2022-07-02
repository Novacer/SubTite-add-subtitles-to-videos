#ifndef SUBTITLER_GUI_AUTO_TRANSCRIBE_AUTO_TRANSCRIBE_WINDOW_H
#define SUBTITLER_GUI_AUTO_TRANSCRIBE_AUTO_TRANSCRIBE_WINDOW_H

#include <QDialog>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)

namespace subtitler {
namespace gui {
namespace auto_transcribe {

/**
 * Contains possible arguments for different kinds of transcribe jobs.
 */
struct Inputs {
    QString video_file;
    QString current_subtitle_file;
    QString login_data;
};

/**
 * Dialog for using cloud services to auto transcribe subtitles.
 */
class AutoTranscribeWindow : public QDialog {
    Q_OBJECT
  public:
    AutoTranscribeWindow(Inputs inputs, QWidget* parent = Q_NULLPTR);
    ~AutoTranscribeWindow();

    void accept() override;
    void reject() override;

    QString OutputFile() const { return output_file_; }

  public slots:
    void onTranscribe();
    void onProgressUpdate(const std::string progress);
    void onTranscribeComplete(QString error);

  private:
    Inputs inputs_;
    QString output_file_;
    QLabel* output_choice_;
    QLabel* progress_;
    QPushButton* transcribe_button_;
    // Disables dialog from being closed during transcribe job.
    bool can_close_;
    QLabel* transcribe_service_explanation_;
};

}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler

#endif
