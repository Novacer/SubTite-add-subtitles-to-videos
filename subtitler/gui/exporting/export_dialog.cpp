#include "subtitler/gui/exporting/export_dialog.h"

#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QThreadPool>
#include <stdexcept>

#include "subtitler/gui/exporting/tasks/burn_subtitle_task.h"
#include "subtitler/gui/exporting/tasks/remux_subtitle_task.h"
#include "subtitler/video/util/video_utils.h"

namespace subtitler {
namespace gui {
namespace exporting {

namespace {

const char *REMUX_SUBTITLE_MESSAGE =
    "Export as mkv. Fastest processing times but may not be supported on all "
    "players";
const char *BURN_SUBTITLE_MESSAGE =
    "Export as mp4. Subtitles are permanently placed (burned) into the video. "
    "Slower processing times but supported on more players";

}  // namespace

ExportWindow::ExportWindow(Inputs inputs, QWidget *parent)
    : QDialog{parent},
      inputs_{inputs},
      can_close_{true},
      export_type_{REMUX_SUBTITLE} {
    setWindowTitle(tr("Export Video"));
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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

    QComboBox *export_type_choice = new QComboBox{this};
    export_type_choice->addItem(tr("Remux as mkv (Recommended)"));
    export_type_choice->addItem(tr("Burn to mp4"));
    export_type_choice->setCurrentIndex(0);
    export_type_choice->setEditable(false);

    export_type_explanation_ = new QLabel{tr(REMUX_SUBTITLE_MESSAGE), this};
    export_type_explanation_->setWordWrap(true);

    QPushButton *choose_output_file =
        new QPushButton{tr("Choose Output Location"), this};
    output_choice_ = new QLabel{this};
    output_choice_->setMinimumWidth(300);

    export_btn_ = new QPushButton{tr("Export"), this};
    progress_ = new QLabel{this};

    QGridLayout *layout = new QGridLayout{this};
    layout->addWidget(input_video_name, 0, 0, 1, 2);
    layout->addWidget(input_subtitle_name, 1, 0, 1, 2);
    layout->addWidget(export_type_choice, 2, 0, 1, 2);
    layout->addWidget(export_type_explanation_, 3, 0, 1, 2);
    layout->addWidget(choose_output_file, 4, 0);
    layout->addWidget(output_choice_, 4, 1);
    layout->addWidget(progress_, 5, 0);
    layout->addWidget(export_btn_, 5, 1, Qt::AlignRight);

    layout->setVerticalSpacing(10);

    connect(choose_output_file, &QPushButton::clicked, this, [this]() {
        std::string filter = "Video Files (*.mp4)";
        QString required_suffix = ".mp4";
        if (export_type_ == ExportType::REMUX_SUBTITLE) {
            filter = "Video Files (*.mkv)";
            required_suffix = ".mkv";
        }

        // On Linux we're using QT 5.12, which getSaveFileName does not
        // add any extension by default. Thus, we create the dialog
        // manually and use setDefaultSuffix instead.
        QFileDialog save_dialog{/* parent= */ this,
                                /* caption= */ tr("Save Video"),
                                /* directory= */ "",
                                /* filter= */ tr(filter.c_str())};
        save_dialog.setDefaultSuffix(required_suffix);
        save_dialog.setAcceptMode(QFileDialog::AcceptSave);
        if (save_dialog.exec() == QDialog::Accepted) {
            output_file_ = save_dialog.selectedFiles().front();
        } else {
            output_file_ = "";
        }

        output_choice_->setText(output_file_);
    });

    connect(export_type_choice,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &ExportWindow::onExportTypeChanged);
    connect(export_btn_, &QPushButton::clicked, this, &ExportWindow::onExport);
}

ExportWindow::~ExportWindow() = default;

void ExportWindow::onExport() {
    if (output_file_.isEmpty()) {
        progress_->setText(tr("Please select output file!"));
        return;
    }

    video_duration_ =
        video::util::GetVideoDuration(inputs_.video_file.toStdString());

    switch (export_type_) {
        case EXPORT_TYPE_UNKNOWN:
            progress_->setText(tr("Unknown export type, try again"));
            return;
        case REMUX_SUBTITLE:
            QThreadPool::globalInstance()->start(new tasks::RemuxSubtitleTask{
                inputs_.video_file, inputs_.subtitle_file, output_file_, this});
            break;
        case BURN_SUBTITLE:
            QThreadPool::globalInstance()->start(new tasks::BurnSubtitleTask{
                inputs_.video_file, inputs_.subtitle_file, output_file_, this});
            break;
    }

    export_btn_->setEnabled(false);
    export_btn_->setVisible(false);
    can_close_ = false;
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
    export_btn_->setEnabled(true);
    export_btn_->setVisible(true);
    can_close_ = true;

    if (!error.isEmpty()) {
        progress_->setText(tr("Error: ") + error);
    }
}

void ExportWindow::accept() {
    if (can_close_) {
        QDialog::accept();
    }
}

void ExportWindow::reject() {
    if (can_close_) {
        QDialog::reject();
    }
}

void ExportWindow::onExportTypeChanged(int index) {
    switch (index) {
        case 0:
            export_type_ = REMUX_SUBTITLE;
            export_type_explanation_->setText(tr(REMUX_SUBTITLE_MESSAGE));
            break;
        case 1:
            export_type_ = BURN_SUBTITLE;
            export_type_explanation_->setText(tr(BURN_SUBTITLE_MESSAGE));
            break;
        default:
            export_type_ = EXPORT_TYPE_UNKNOWN;
            export_type_explanation_->setText("");
            break;
    }

    output_file_ = "";
    output_choice_->setText(output_file_);
}

}  // namespace exporting
}  // namespace gui
}  // namespace subtitler
