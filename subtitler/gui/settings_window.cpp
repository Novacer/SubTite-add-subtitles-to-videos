#include "subtitler/gui/settings_window.h"

#include <QDebug>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

namespace subtitler {
namespace gui {

Settings GetSettings(const Settings &current_settings) {
    Settings new_settings = current_settings;
    SettingsWindow setting_dialog{new_settings};
    setting_dialog.exec();
    return new_settings;
}

SettingsWindow::SettingsWindow(Settings &settings, QWidget *parent)
    : QDialog{parent}, settings_{settings} {
    setWindowTitle("Choose Files");

    QPushButton *choose_video = new QPushButton{tr("Choose Video File"), this};
    QPushButton *choose_subtitle =
        new QPushButton{tr("Choose Subtitle File"), this};
    QPushButton *close = new QPushButton{tr("Done"), this};

    QLabel *video_choice = new QLabel{this};
    video_choice->setMinimumWidth(300);
    QLabel *subtitle_choice = new QLabel{this};
    subtitle_choice->setMinimumWidth(300);
    error_msg_ = new QLabel{this};
    error_msg_->setWordWrap(true);

    QGridLayout *layout = new QGridLayout{this};
    layout->addWidget(choose_video, 0, 0);
    layout->addWidget(video_choice, 0, 1);
    layout->addWidget(choose_subtitle, 1, 0);
    layout->addWidget(subtitle_choice, 1, 1);
    layout->addWidget(error_msg_, 2, 0);
    layout->addWidget(close, 2, 1, Qt::AlignRight);

    layout->setVerticalSpacing(10);

    connect(choose_video, &QPushButton::clicked, this, [this, video_choice]() {
        settings_.video_file = QFileDialog::getOpenFileName(
            /* parent= */ this,
            /* caption= */ tr("Open Video"),
            /* directory= */ "",
            /* filter= */ tr("Video Files (*.mp4)"));
        video_choice->setText(settings_.video_file);
    });

    connect(choose_subtitle, &QPushButton::clicked, this,
            [this, subtitle_choice]() {
                settings_.subtitle_file = QFileDialog::getSaveFileName(
                    /* parent= */ this,
                    /* caption= */ tr("Create/Open Subtitle File"),
                    /* directory= */ "",
                    /* filter= */ tr("SRT Files (*.srt)"));
                subtitle_choice->setText(settings_.subtitle_file);
            });

    connect(close, &QPushButton::clicked, this, [this]() {
        if (settings_.video_file.isEmpty()) {
            error_msg_->setText(tr("Choose video file!"));
        } else if (settings_.subtitle_file.isEmpty()) {
            error_msg_->setText(tr("Choose subtitle file!"));
        } else {
            this->done(0);
        }
    });
}

}  // namespace gui
}  // namespace subtitler
