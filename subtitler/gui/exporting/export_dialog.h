#ifndef SUBTITLER_GUI_EXPORTING_EXPORT_DIALOG_H
#define SUBTITLER_GUI_EXPORTING_EXPORT_DIALOG_H

#include <QDialog>
#include <QString>
#include <memory>
#include <atomic>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)

namespace subtitler::video::processing {

class FFMpeg;

}  // namespace subtitler::video::processing

namespace subtitler {
namespace gui {
namespace exporting {

struct Inputs {
    QString video_file;
    QString subtitle_file;
};

class ExportWindow : public QDialog {
    Q_OBJECT
  public:
    ExportWindow(Inputs inputs, QWidget* parent = Q_NULLPTR);
    ~ExportWindow();

  public slots:
    void onExport();

  private:
    Inputs inputs_;
    QString output_file_;
    QLabel* progress_;
    QPushButton* export_btn_;
    std::unique_ptr<video::processing::FFMpeg> ffmpeg_;
};

}  // namespace exporting
}  // namespace gui
}  // namespace subtitler

#endif
