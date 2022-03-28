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
#include "subtitler/video/processing/progress_parser.h"
#include "subtitler/video/util/video_utils.h"

namespace subtitler {
namespace gui {
namespace exporting {

namespace {

class FFMpegBurnSubtitleTask : public QRunnable {
  public:
    FFMpegBurnSubtitleTask(QString video, QString subtitle, QString output,
                           QLabel *progress)
        : QRunnable{},
          video_{video},
          subtitle_{subtitle},
          output_{output},
          progress_{progress} {}

    void run() override {
        const auto duration =
            video::util::GetVideoDuration(video_.toStdString());

        std::string ffmpeg_path =
            QCoreApplication::applicationDirPath().toStdString() + "/ffmpeg";

        video::processing::FFMpeg ffmpeg{
            ffmpeg_path, std::make_unique<subprocess::SubprocessExecutor>()};

        try {
            ffmpeg.BurnSubtitlesAsync(
                video_.toStdString(), subtitle_.toStdString(),
                output_.toStdString(),
                [this, duration](const video::processing::Progress &progress) {
                    if (progress.progress == "continue") {
                        auto percentage = progress.out_time_us * 100 / duration;
                        QMetaObject::invokeMethod(
                            progress_, "setText",
                            Q_ARG(QString,
                                  QString::number(percentage) + "% Complete"));
                    } else if (progress.progress == "end") {
                        QMetaObject::invokeMethod(progress_, "setText",
                                                  Q_ARG(QString, "Complete!"));
                    }
                });
            ffmpeg.WaitForAsyncTask();
        } catch (const std::exception &e) {
            qDebug() << "Error starting ffmpeg: " << e.what();
        }
    }

  private:
    QString video_;
    QString subtitle_;
    QString output_;
    QLabel *progress_;
};

}  // namespace

ExportWindow::ExportWindow(Inputs inputs, QWidget *parent)
    : QDialog{parent}, inputs_{std::move(inputs)} {
    setWindowTitle(tr("Export Video"));

    if (inputs_.video_file.isEmpty()) {
        throw std::runtime_error{"Cannot export empty video path!"};
    }
    QLabel *input_video_name = new QLabel{tr("Using: ") + inputs_.video_file};

    QLabel *input_subtitle_name = new QLabel{tr("Using: None"), this};
    if (!inputs_.subtitle_file.isEmpty()) {
        input_subtitle_name->setText(tr("Using: ") + inputs_.subtitle_file);
    }

    QPushButton *choose_output_file =
        new QPushButton{tr("Choose Output Location"), this};
    QLabel *output_choice = new QLabel{this};
    output_choice->setMinimumWidth(300);

    export_btn_ = new QPushButton{tr("Export"), this};
    progress_ = new QLabel{this};
    progress_->hide();

    QGridLayout *layout = new QGridLayout{this};
    layout->addWidget(input_video_name, 0, 0);
    layout->addWidget(input_subtitle_name, 1, 0);
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
    export_btn_->setEnabled(false);
    progress_->show();

    QThreadPool::globalInstance()->start(new FFMpegBurnSubtitleTask{
        inputs_.video_file, inputs_.subtitle_file, output_file_, progress_});
}

}  // namespace exporting
}  // namespace gui
}  // namespace subtitler
