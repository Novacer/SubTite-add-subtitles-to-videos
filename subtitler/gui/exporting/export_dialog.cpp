#include "subtitler/gui/exporting/export_dialog.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRunnable>
#include <QThreadPool>
#include <stdexcept>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/processing/ffmpeg.h"
#include "subtitler/video/util/video_utils.h"

// Reference https://stackoverflow.com/a/14713932/17786559
// Both Q_DECLARE_METATYPE and qRegisterMetaType in the ctor are needed
// so that QT can pass Progress through QMetaObject::invokeMethod
Q_DECLARE_METATYPE(subtitler::video::processing::Progress)

namespace subtitler {
namespace gui {
namespace exporting {

namespace {

class FFMpegBurnSubtitleTask : public QRunnable {
  public:
    FFMpegBurnSubtitleTask(QString video, QString subtitle, QString output,
                           ExportWindow *parent)
        : QRunnable{},
          video_{video},
          subtitle_{subtitle},
          output_{output},
          parent_{parent} {
        qRegisterMetaType<subtitler::video::processing::Progress>();
    }

    void run() override {
        std::string ffmpeg_path =
            QCoreApplication::applicationDirPath().toStdString() + "/ffmpeg";

        video::processing::FFMpeg ffmpeg{
            ffmpeg_path, std::make_unique<subprocess::SubprocessExecutor>()};

        try {
            ffmpeg.BurnSubtitlesAsync(
                video_.toStdString(), subtitle_.toStdString(),
                output_.toStdString(),
                [this](const video::processing::Progress &progress) {
                    QMetaObject::invokeMethod(
                        parent_, "onProgressUpdate",
                        // Q_ARG expects fully qualified name.
                        Q_ARG(subtitler::video::processing::Progress,
                              progress));
                });
            ffmpeg.WaitForAsyncTask();
            QMetaObject::invokeMethod(parent_, "onExportComplete",
                                      Q_ARG(QString, ""));
        } catch (const std::exception &e) {
            qDebug() << "Error starting ffmpeg: " << e.what();
            QMetaObject::invokeMethod(parent_, "onExportComplete",
                                      Q_ARG(QString, e.what()));
        }
    }

  private:
    QString video_;
    QString subtitle_;
    QString output_;
    ExportWindow *parent_;
};

}  // namespace

ExportWindow::ExportWindow(Inputs inputs, QWidget *parent)
    : QDialog{parent}, inputs_{std::move(inputs)} {
    setWindowTitle(tr("Export Video"));

    if (inputs_.video_file.isEmpty()) {
        throw std::runtime_error{"Cannot export empty video path!"};
    }
    QLabel *input_video_name =
        new QLabel{tr("Video File: ") + inputs_.video_file};
    input_video_name->setFrameStyle(QFrame::Panel | QFrame::Plain);

    QLabel *input_subtitle_name = new QLabel{tr("Subtitle File: None"), this};
    if (!inputs_.subtitle_file.isEmpty()) {
        input_subtitle_name->setText(tr("Subtitle File: ") +
                                     inputs_.subtitle_file);
    }
    input_subtitle_name->setFrameStyle(QFrame::Panel | QFrame::Plain);

    QPushButton *choose_output_file =
        new QPushButton{tr("Choose Output Location"), this};
    QLabel *output_choice = new QLabel{this};
    output_choice->setMinimumWidth(300);

    export_btn_ = new QPushButton{tr("Export"), this};
    progress_ = new QLabel{this};

    QGridLayout *layout = new QGridLayout{this};
    layout->addWidget(input_video_name, 0, 0, 1, 2);
    layout->addWidget(input_subtitle_name, 1, 0, 1, 2);
    layout->addWidget(choose_output_file, 2, 0);
    layout->addWidget(output_choice, 2, 1);
    layout->addWidget(progress_, 3, 0);
    layout->addWidget(export_btn_, 3, 1, Qt::AlignRight);

    layout->setVerticalSpacing(5);

    // TODO: add drop down to choose the video encoding mode
    // (remux vs transcode)

    connect(choose_output_file, &QPushButton::clicked, this,
            [this, output_choice]() {
                output_file_ = QFileDialog::getSaveFileName(
                    /* parent= */ this,
                    /* caption= */ tr("Save Video"),
                    /* directory= */ "",
                    /* filter= */ tr("Video Files (*.mp4)"));
                output_choice->setText(output_file_);
            });

    connect(export_btn_, &QPushButton::clicked, this, &ExportWindow::onExport);
}

ExportWindow::~ExportWindow(){};

void ExportWindow::onExport() {
    if (output_file_.isEmpty()) {
        progress_->setText(tr("Please select output file!"));
        return;
    }

    video_duration_ =
        video::util::GetVideoDuration(inputs_.video_file.toStdString());

    export_btn_->setEnabled(false);
    export_btn_->setVisible(false);

    QThreadPool::globalInstance()->start(new FFMpegBurnSubtitleTask{
        inputs_.video_file, inputs_.subtitle_file, output_file_, this});
}

void ExportWindow::onProgressUpdate(
    const video::processing::Progress progress_data) {
    using namespace std::chrono_literals;

    if (progress_data.progress == "continue") {
        if (video_duration_ > 0ns) {
            auto percentage = progress_data.out_time_us * 100 / video_duration_;
            progress_->setText(QString::number(percentage) + tr("% Complete"));
        }

    } else if (progress_data.progress == "end") {
        progress_->setText("Completed!");
    }
}

void ExportWindow::onExportComplete(QString error) {
    export_btn_->setVisible(true);
    export_btn_->setEnabled(true);

    if (!error.isEmpty()) {
        progress_->setText(tr("Error: ") + error);
    }
}

}  // namespace exporting
}  // namespace gui
}  // namespace subtitler
