#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QThreadPool>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "subtitler/gui/auto_transcribe/auto_transcribe_window.h"
#include "subtitler/gui/auto_transcribe/tasks/transcribe_task.h"
#include "subtitler/speech_recognition/auto_transcriber.h"

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
    choose_output_file_btn_ =
        new QPushButton{tr("Choose Output Location"), this};
    output_choice_ = new QLabel{this};
    output_choice_->setMinimumWidth(300);
    output_choice_->setText(output_file_);

    transcribe_button_ = new QPushButton{tr("Transcribe"), this};
    progress_ = new QLabel{this};
    progress_->setMaximumWidth(300);

    QGridLayout *layout = new QGridLayout{this};
    layout->addWidget(input_video_name, 0, 0, 1, 2);
    layout->addWidget(transcribe_service_explanation_, 2, 0, 1, 2);
    layout->addWidget(choose_output_file_btn_, 4, 0);
    layout->addWidget(output_choice_, 4, 1);
    layout->addWidget(progress_, 5, 0);
    layout->addWidget(transcribe_button_, 5, 1, Qt::AlignRight);

    layout->setVerticalSpacing(10);

    connect(choose_output_file_btn_, &QPushButton::clicked, this, [this]() {
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
    if (output_file_.isEmpty()) {
        progress_->setText(tr("Please select output file!"));
    }

    nlohmann::json login_json =
        nlohmann::json::parse(inputs_.login_data.toStdString());
    QString api_key = QString::fromStdString(login_json.at("api-key"));
    QString api_region = QString::fromStdString(login_json.at("api-region"));

    QThreadPool::globalInstance()->start(new tasks::TranscribeTask{
        api_key, api_region, inputs_.video_file, output_file_, this});

    transcribe_button_->setEnabled(false);
    transcribe_button_->setVisible(false);
    choose_output_file_btn_->setEnabled(false);
    can_close_ = false;
}

void AutoTranscribeWindow::onProgressUpdate(const QString progress) {
    progress_->setText(progress);
}

void AutoTranscribeWindow::onTranscribeComplete(QString error) {
    can_close_ = true;
    if (error.isEmpty()) {
        accept();
    } else {
        progress_->setText(error);
    }
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
