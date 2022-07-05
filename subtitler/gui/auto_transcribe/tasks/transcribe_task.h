#ifndef SUBTITLER_GUI_AUTO_TRANSCRIBE_TASKS_TRANSCRIBE_TASK
#define SUBTITLER_GUI_AUTO_TRANSCRIBE_TASKS_TRANSCRIBE_TASK

#include <QRunnable>
#include <QString>

namespace subtitler::gui::auto_transcribe {

class AutoTranscribeWindow;

}

namespace subtitler {
namespace gui {
namespace auto_transcribe {
namespace tasks {

class TranscribeTask : public QRunnable {
  public:
    TranscribeTask(QString api_key, QString api_region, QString input_video,
                   QString output_srt, AutoTranscribeWindow *parent);

    void run() override;

  private:
    QString api_key_;
    QString api_region_;
    QString input_video_;
    QString output_srt_;
    AutoTranscribeWindow *parent_;
};

}  // namespace tasks
}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler

#endif
