#include "subtitler/gui/main_window.h"

#include <QtAVPlayer/qavaudiooutput.h>
#include <QtAVPlayer/qavplayer.h>
#include <QtAVPlayer/qavvideoframe.h>

#include <QAbstractVideoSurface>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QVBoxLayout>
#include <chrono>
#include <optional>
#include <stdexcept>

#include "subtitler/gui/auto_transcribe/auto_transcribe_window.h"
#include "subtitler/gui/auto_transcribe/login/login_msc.h"
#include "subtitler/gui/exporting/export_dialog.h"
#include "subtitler/gui/player_controls/play_button.h"
#include "subtitler/gui/player_controls/step_button.h"
#include "subtitler/gui/settings_window.h"
#include "subtitler/gui/subtitle_editor/subtitle_editor.h"
#include "subtitler/gui/timeline/timeline.h"
#include "subtitler/gui/timeline/timer.h"
#include "subtitler/gui/video_renderer/opengl_renderer.h"
#include "subtitler/video/processing/downscaling.h"
#include "subtitler/video/util/video_utils.h"

namespace subtitler {
namespace gui {

MainWindow::~MainWindow() = default;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("SubTite");
    setMinimumSize(1280, 720);
    QWidget *placeholder = new QWidget{this};
    QVBoxLayout *layout = new QVBoxLayout(placeholder);

    Settings settings = GetSettings(Settings{});

    if (settings.video_file.isEmpty()) {
        qDebug() << "No video file selected";
        throw std::runtime_error{"No video file was selected"};
    }

    if (settings.subtitle_file.isEmpty()) {
        qDebug() << "No output file selected";
        throw std::runtime_error{"No subtitle file was selected"};
    }

    subtitle_file_ = settings.subtitle_file;

    video_file_ = std::make_unique<QFile>(settings.video_file);
    if (!video_file_->open(QFile::ReadOnly)) {
        qDebug() << "Video file could not be opened";
        throw std::runtime_error{"Invalid video file path"};
    }

    QMenu *file_menu = menuBar()->addMenu(tr("&File"));
    QAction *export_video_action = file_menu->addAction(tr("Export Video"));
    QMenu *subtitle_menu = menuBar()->addMenu(tr("&Subtitle"));
    QAction *auto_transcribe_action =
        subtitle_menu->addAction(tr("Auto Transcribe"));

    video_renderer_ = new video_renderer::OpenGLRenderer(placeholder);

    player_ = std::make_unique<QAVPlayer>();
    player_->setSource(video_file_->fileName(), video_file_.get());

    QWidget *interactive_elements_placeholder = new QWidget{this};
    QWidget *player_controls_placeholder =
        new QWidget{interactive_elements_placeholder};
    QHBoxLayout *player_controls_layout =
        new QHBoxLayout{player_controls_placeholder};

    auto *step_backwards =
        new player_controls::StepBackwardsButton{player_controls_placeholder};
    auto *play_button =
        new player_controls::PlayButton(player_controls_placeholder);
    auto *step_forwards =
        new player_controls::StepForwardsButton{player_controls_placeholder};

    player_controls_layout->addWidget(step_backwards);
    player_controls_layout->addWidget(play_button);
    player_controls_layout->addWidget(step_forwards);

    timeline::Timer *timer =
        new timeline::Timer{interactive_elements_placeholder};

    auto duration_us =
        video::util::GetVideoDuration(video_file_->fileName().toStdString());

    std::chrono::milliseconds duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration_us);
    timeline::Timeline *timeline = new timeline::Timeline{
        duration, subtitle_file_, interactive_elements_placeholder};

    QVBoxLayout *interactive_elements_layout =
        new QVBoxLayout{interactive_elements_placeholder};
    interactive_elements_layout->addWidget(player_controls_placeholder);
    interactive_elements_layout->addWidget(timer);
    interactive_elements_layout->addWidget(timeline);

    layout->addWidget(video_renderer_, 60);
    layout->addWidget(interactive_elements_placeholder, 40, Qt::AlignBottom);

    setCentralWidget(placeholder);

    editor_ = new subtitle_editor::SubtitleEditor{this};
    editor_->setWindowTitle(tr("Subtitle Editor"));
    editor_->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, editor_);

    export_dialog_ = Q_NULLPTR;
    auto_transcribe_window_ = Q_NULLPTR;
    login_window_ = Q_NULLPTR;

    // Play/Pause/Step connections
    connect(play_button, &player_controls::PlayButton::play, player_.get(),
            &QAVPlayer::play);
    connect(play_button, &player_controls::PlayButton::pause, player_.get(),
            &QAVPlayer::pause);
    connect(player_.get(), &QAVPlayer::played, timeline,
            &timeline::Timeline::onPlayerPlay);
    connect(player_.get(), &QAVPlayer::paused, timeline,
            &timeline::Timeline::onPlayerPause);
    connect(step_backwards, &player_controls::StepBackwardsButton::stepDelta,
            timeline, &timeline::Timeline::onUserStepChangedTime);
    connect(step_forwards, &player_controls::StepForwardsButton::stepDelta,
            timeline, &timeline::Timeline::onUserStepChangedTime);

    // Connections to synchronize time between ruler, player, and timer.
    connect(timeline, &timeline::Timeline::rulerChangedTime, timer,
            &timeline::Timer::onTimerChanged);
    connect(timeline, &timeline::Timeline::userDraggedRulerChangeTime, this,
            &MainWindow::onRulerChangedTime);
    connect(this, &MainWindow::playerChangedTime, timeline,
            &timeline::Timeline::onPlayerChangedTime);

    audio_output_ = std::make_unique<QAVAudioOutput>();
    // Handle decoded frames.
    connect(player_.get(), &QAVPlayer::audioFrame, this,
            &MainWindow::onAudioFrameDecoded);
    connect(player_.get(), &QAVPlayer::videoFrame, this,
            &MainWindow::onVideoFrameDecoded);

    // Handle changes to subtitle editor state.
    connect(timeline, &timeline::Timeline::openSubtitleEditor, editor_,
            &subtitle_editor::SubtitleEditor::onOpenSubtitle);
    connect(timeline, &timeline::Timeline::changeSubtitleStartEndTime, editor_,
            &subtitle_editor::SubtitleEditor::onSubtitleChangeStartEndTime);
    connect(timeline, &timeline::Timeline::changeSubtitleStartEndTimeFinished,
            editor_, &subtitle_editor::SubtitleEditor::onSave);
    connect(this, &MainWindow::subtitleFileReload, editor_,
            &subtitle_editor::SubtitleEditor::onSubtitleFileReload);

    // Handles changes to subtitle file.
    connect(editor_, &subtitle_editor::SubtitleEditor::saved, this,
            &MainWindow::onSubtitleFileChanged);
    connect(timeline, &timeline::Timeline::subtitleFileLoaded, this,
            &MainWindow::onSubtitleFileChanged);
    connect(this, &MainWindow::subtitleFileReload, timeline,
            &timeline::Timeline::onSubtitleFileReload);

    // Handles top menu bar actions
    auto pause_player = [play_button](bool checked) {
        // If the player is playing, then automatically click the pause button.
        if (play_button->is_playing()) {
            play_button->onClick();
        }
    };
    connect(export_video_action, &QAction::triggered, this,
            &MainWindow::onExport);
    connect(export_video_action, &QAction::triggered, this, pause_player);
    connect(auto_transcribe_action, &QAction::triggered, this,
            &MainWindow::onAutoTranscribe);
    connect(auto_transcribe_action, &QAction::triggered, this, pause_player);

    if (!subtitle_file_.isEmpty()) {
        timeline->LoadSubtitles();
    }

    user_seeked_ = false;

    // Init the first frame.
    player_->pause();
}

void MainWindow::onRulerChangedTime(std::chrono::milliseconds ms) {
    user_seeked_ = true;
    player_->seek(ms.count());
}

void MainWindow::onAudioFrameDecoded(const QAVAudioFrame &audio_frame) {
    if (player_->state() == QAVPlayer::State::PlayingState) {
        audio_output_->play(audio_frame);
    }
}

void MainWindow::onVideoFrameDecoded(const QAVVideoFrame &video_frame) {
    if (video_renderer_ == nullptr) {
        return;
    }
    QVideoFrame rgb_video_frame = video_frame.convertTo(AV_PIX_FMT_RGB32);
    video_renderer_->displayFrame(rgb_video_frame);

    std::chrono::milliseconds ms{(quint64)(video_frame.pts() * 1000)};
    if (!user_seeked_) {
        // Only emit if the user did not seek.
        // If user seeked, then the player is not changing the time!
        emit this->playerChangedTime(ms);
    } else {
        user_seeked_ = false;
    }

    // This is the first decoded frame.
    if (!downscale_filter_.has_value()) {
        struct video::processing::InputVideoScalingInfo input_scaling_info {
            rgb_video_frame.height(), rgb_video_frame.width(),
                /*fps=*/1 / player_->videoFrameRate()
        };
        qDebug() << "Video height:" << input_scaling_info.height
                 << "width:" << input_scaling_info.width
                 << "fps:" << input_scaling_info.fps;

        downscale_filter_ = QString::fromStdString(
            video::processing::GetFFMpegScaleFilterRecommendation(
                input_scaling_info));
        if (!downscale_filter_->isEmpty()) {
            QList<QString> filters{*downscale_filter_, "acopy"};
            player_->setFilters(std::move(filters));
        }
    }
}

void MainWindow::onSubtitleFileChanged(std::size_t num_loaded) {
    player_->setFilter("");
    if (num_loaded == 0 || subtitle_file_.isEmpty()) {
        return;
    }
    QString escaped_path = subtitle_file_;
    escaped_path.replace(":", "\\:");

    QString video_filter;
    if (downscale_filter_.has_value() && !downscale_filter_->isEmpty()) {
        video_filter =
            QString("%1,subtitles='%2'").arg(*downscale_filter_, escaped_path);
    } else {
        video_filter = QString("subtitles='%1'").arg(escaped_path);
    }

    QList<QString> filters{std::move(video_filter), "acopy"};
    player_->setFilters(std::move(filters));
}

void MainWindow::onExport(bool checked) {
    if (export_dialog_) {
        return;
    }
    exporting::Inputs inputs;
    inputs.video_file = video_file_->fileName();
    inputs.subtitle_file = subtitle_file_;
    export_dialog_ = new exporting::ExportWindow{std::move(inputs), this};
    // WA_DeleteOnClose will delete dialog when done() is called.
    export_dialog_->setAttribute(Qt::WA_DeleteOnClose);
    export_dialog_->open();
    connect(export_dialog_, &QDialog::finished,
            [this](int result) { export_dialog_ = Q_NULLPTR; });
}

void MainWindow::onSubtitleFileReload(const QString &new_subtitle_file) {
    subtitle_file_ = new_subtitle_file;
    emit subtitleFileReload(subtitle_file_);
}

void MainWindow::onAutoTranscribe(bool checked) {
    if (login_window_ || auto_transcribe_window_) {
        return;
    }

    login_window_ =
        new auto_transcribe::login::LoginMicrosoftCognitiveServicesWindow{this};
    login_window_->open();

    connect(login_window_, &QDialog::finished, [this](int result) {
        if (result == QDialog::Accepted &&
            !login_window_->GetLoginData().isEmpty()) {
            QString login_data = login_window_->GetLoginData();

            auto_transcribe::Inputs inputs;
            inputs.video_file = video_file_->fileName();
            inputs.current_subtitle_file = subtitle_file_;
            inputs.login_data = login_data;

            auto_transcribe_window_ = new auto_transcribe::AutoTranscribeWindow{
                std::move(inputs), this};
            auto_transcribe_window_->open();

            connect(auto_transcribe_window_, &QDialog::finished,
                    [this](int result) {
                        if (result == QDialog::Accepted) {
                            QString new_file =
                                auto_transcribe_window_->OutputFile();
                            onSubtitleFileReload(new_file);
                        }
                        delete auto_transcribe_window_;
                        auto_transcribe_window_ = Q_NULLPTR;
                    });
        }
        delete login_window_;
        login_window_ = Q_NULLPTR;
    });
}

}  // namespace gui
}  // namespace subtitler
