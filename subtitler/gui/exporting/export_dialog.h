#ifndef SUBTITLER_GUI_EXPORTING_EXPORT_DIALOG_H
#define SUBTITLER_GUI_EXPORTING_EXPORT_DIALOG_H

#include <QDialog>
#include <QString>
#include <chrono>
#include <memory>

#include "subtitler/video/processing/progress_parser.h"

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)

namespace subtitler::video::processing {

class FFMpeg;

}  // namespace subtitler::video::processing

namespace subtitler {
namespace gui {
namespace exporting {

/**
 * Contains possible import arguments for different kinds of export jobs.
 */
struct Inputs {
    QString video_file;
    QString subtitle_file;
};

/**
 * Dialog for exporting edited videos and running async render jobs.
 */
class ExportWindow : public QDialog {
    Q_OBJECT
  public:
    ExportWindow(Inputs inputs, QWidget* parent = Q_NULLPTR);
    ~ExportWindow();

  public slots:
    void onExport();
    void onProgressUpdate(
        const subtitler::video::processing::Progress progress);
    void onExportComplete(QString error);

  private:
    Inputs inputs_;
    QString output_file_;
    QLabel* progress_;
    QPushButton* export_btn_;
    std::unique_ptr<video::processing::FFMpeg> ffmpeg_;
    std::chrono::microseconds video_duration_;
};

}  // namespace exporting
}  // namespace gui
}  // namespace subtitler

#endif
