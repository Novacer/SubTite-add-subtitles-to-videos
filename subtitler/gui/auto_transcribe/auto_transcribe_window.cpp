#include "subtitler/gui/auto_transcribe/auto_transcribe_window.h"

#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QThreadPool>
#include <stdexcept>

namespace subtitler {
namespace gui {
namespace auto_transcribe {

namespace {

const char *TRANSCRIBE_SERVICE_EXPLANATION =
    "Uses Microsoft Cognitive Services to auto-transcribe the subtitles";

}  // namespace

AutoTranscribeWindow::AutoTranscribeWindow(Inputs inputs, QWidget *parent)
    : QDialog{parent}, inputs_{inputs}, can_close_{true} {
    setWindowTitle(tr("Auto Transcribe"));
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (inputs_.video_file.isEmpty()) {
        throw std::runtime_error{"Cannot export empty video path!"};
    }
    QLabel *input_video_name =
        new QLabel{tr("Video File: ") + inputs_.video_file};
    input_video_name->setFrameStyle(QFrame::Panel | QFrame::Plain);

    transcribe_service_explanation_ =
        new QLabel{tr(TRANSCRIBE_SERVICE_EXPLANATION), this};
    transcribe_service_explanation_->setWordWrap(true);

    output_file_ = inputs_.current_subtitle_file;
    QPushButton *choose_output_file =
        new QPushButton{tr("Choose Output Location"), this};
    output_choice_ = new QLabel{this};
    output_choice_->setMinimumWidth(300);
    output_choice_->setText(output_file_);

    transcribe_button_ = new QPushButton{tr("Transcribe"), this};
    progress_ = new QLabel{this};

    QGridLayout *layout = new QGridLayout{this};
    layout->addWidget(input_video_name, 0, 0, 1, 2);
    layout->addWidget(transcribe_service_explanation_, 2, 0, 1, 2);
    layout->addWidget(choose_output_file, 4, 0);
    layout->addWidget(output_choice_, 4, 1);
    layout->addWidget(progress_, 5, 0);
    layout->addWidget(transcribe_button_, 5, 1, Qt::AlignRight);

    layout->setVerticalSpacing(10);

    connect(choose_output_file, &QPushButton::clicked, this, [this]() {
        std::string filter = "SRT Files (*.srt)";
        QString required_suffix = ".srt";

        // On Linux we're using QT 5.12, which getSaveFileName does not
        // add any extension by default. Thus, we create the dialog
        // manually and use setDefaultSuffix instead.
        QFileDialog save_dialog{/* parent= */ this,
                                /* caption= */ tr("Save Subtitles"),
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

    connect(transcribe_button_, &QPushButton::clicked, this,
            &AutoTranscribeWindow::onTranscribe);
}

AutoTranscribeWindow::~AutoTranscribeWindow() = default;

void AutoTranscribeWindow::onTranscribe() {
    // TODO
    // ...
    onTranscribeComplete("");
}

void AutoTranscribeWindow::onProgressUpdate(const std::string progress) {
    // TODO
}

void AutoTranscribeWindow::onTranscribeComplete(QString error) {
    // TODO something with error
    can_close_ = true;
    accept();
}

void AutoTranscribeWindow::accept() {
    if (can_close_) {
        QDialog::accept();
    }
}

void AutoTranscribeWindow::reject() {
    if (can_close_) {
        QDialog::reject();
    }
}

}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler
