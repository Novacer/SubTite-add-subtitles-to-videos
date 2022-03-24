#include "subtitler/gui/main_window.h"

extern "C" {
#include <libavformat/avformat.h>
}

#include <QtAVPlayer/qavaudiooutput.h>
#include <QtAVPlayer/qavplayer.h>
#include <QtAVPlayer/qavvideoframe.h>

#include <QAbstractVideoSurface>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QGLWidget>
#include <QHBoxLayout>
#include <QMediaObject>
#include <QMediaService>
#include <QVBoxLayout>
#include <QVideoRendererControl>
#include <QVideoSurfaceFormat>
#include <QVideoWidget>
#include <chrono>
#include <iostream>

#include "subtitler/gui/player_controls/play_button.h"
#include "subtitler/gui/player_controls/step_button.h"
#include "subtitler/gui/subtitle_editor/subtitle_editor.h"
#include "subtitler/gui/timeline/timeline.h"
#include "subtitler/gui/timeline/timer.h"

namespace subtitler {
namespace gui {

class QGLCanvas : public QGLWidget {
  public:
    QGLCanvas(QWidget *parent = NULL);
    void setImage(const QImage &image);

  protected:
    void paintEvent(QPaintEvent *) override;
    QRect centeredViewport(int width, int height);

  private:
    QImage img_;
    bool set_;
};

QGLCanvas::QGLCanvas(QWidget *parent) : QGLWidget(parent), set_{false} {
    QSizePolicy sp = this->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Preferred);
    sp.setVerticalPolicy(QSizePolicy::Preferred);
    sp.setHeightForWidth(true);
    this->setSizePolicy(sp);
}

void QGLCanvas::setImage(const QImage &image) {
    img_ = image;
    set_ = true;
}

QRect QGLCanvas::centeredViewport(int width, int height) {
    double aspectRatio = 1.0;
    if (set_) {
        aspectRatio = (double) img_.width() / (double) img_.height();
    }
    int heightFromWidth = (int)(width / aspectRatio);
    int widthFromHeight = (int)(height * aspectRatio);

    if (heightFromWidth <= height) {
        return QRect(0, (height - heightFromWidth) / 2, width, heightFromWidth);
    } else {
        return QRect((width - widthFromHeight) / 2.0, 0, widthFromHeight,
                     height);
    }
}

void QGLCanvas::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setViewport(centeredViewport(width(), height()));
    // Set the painter to use a smooth scaling algorithm.
    p.setRenderHint(QPainter::SmoothPixmapTransform, 1);

    p.drawImage(QRect(QPoint(0,0), size()), img_);
}

namespace {}  // namespace

MainWindow::~MainWindow() = default;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("SubTite");
    setMinimumSize(1280, 500);
    QWidget *placeholder = new QWidget{this};
    QVBoxLayout *layout = new QVBoxLayout(placeholder);

    video_canvas_ = new QGLCanvas(placeholder);

    player_ = std::make_unique<QAVPlayer>();

    QString file_name = QFileDialog::getOpenFileName(
        /* parent= */ this,
        /* caption= */ tr("Open Video"),
        /* directory= */ "",
        /* filter= */ tr("Video Files (*.mp4)"));

    QString output_name = QFileDialog::getSaveFileName(
        /* parent= */ this,
        /* caption= */ tr("Create/Open Subtitle File"),
        /* directory= */ "",
        /* filter= */ tr("SRT Files (*.srt)"));

    if (file_name.isEmpty()) {
        qDebug() << "No video file selected";
        QCoreApplication::exit(1);
    }

    if (output_name.isEmpty()) {
        qDebug() << "No output file selected";
    }

    subtitle_file_ = output_name;

    video_file_ = std::make_unique<QFile>(file_name);
    if (!video_file_->open(QFile::ReadOnly)) {
        qDebug() << "Video file could not be opened";
        QCoreApplication::exit(1);
    }
    player_->setSource(file_name, video_file_.get());

    QWidget *player_controls_placeholder = new QWidget{this};
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

    timeline::Timer *timer = new timeline::Timer{placeholder};

    // Get video duration
    // TODO: make util class?
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    auto file_stdstr = file_name.toStdString();
    avformat_open_input(&pFormatCtx, file_stdstr.c_str(), NULL, NULL);
    avformat_find_stream_info(pFormatCtx, NULL);
    auto duration_us = pFormatCtx->duration;
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

    std::chrono::milliseconds duration{duration_us / 1000};
    timeline::Timeline *timeline =
        new timeline::Timeline{duration, output_name, placeholder};

    layout->addWidget(video_canvas_, 60);
    layout->addWidget(player_controls_placeholder);
    layout->addWidget(timer);
    layout->addWidget(timeline);

    setCentralWidget(placeholder);

    editor_ = new subtitle_editor::SubtitleEditor{this};
    editor_->setWindowTitle(tr("Subtitle Editor"));
    editor_->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, editor_);

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

    // Handles changes to subtitle file.
    connect(editor_, &subtitle_editor::SubtitleEditor::saved, this,
            &MainWindow::onSubtitleFileChanged);
    connect(timeline, &timeline::Timeline::subtitleFileLoaded, this,
            &MainWindow::onSubtitleFileChanged);

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
    if (video_canvas_ == nullptr) return;
    QVideoFrame videoFrame = video_frame.convertTo(AV_PIX_FMT_RGB32);
    // if (!video_renderer_->m_surface->isActive() ||
    //     video_renderer_->m_surface->surfaceFormat().frameSize() !=
    //         videoFrame.size()) {
    //     QVideoSurfaceFormat f(videoFrame.size(), videoFrame.pixelFormat(),
    //                           videoFrame.handleType());
    //     video_renderer_->m_surface->start(f);
    // }
    video_canvas_->setImage(videoFrame.image());
    video_canvas_->update();
    std::chrono::milliseconds ms{(quint64)(video_frame.pts() * 1000)};
    if (!user_seeked_) {
        // Only emit if the user did not seek.
        // If user seeked, then the player is not changing the time!
        emit this->playerChangedTime(ms);
    } else {
        user_seeked_ = false;
    }
}

void MainWindow::onSubtitleFileChanged(std::size_t num_loaded) {
    player_->setFilter("");
    if (num_loaded == 0) {
        return;
    }
    QString escaped_path = subtitle_file_;
    escaped_path.replace(":", "\\:");
    player_->setFilter("subtitles='" + escaped_path + "'");
}

}  // namespace gui
}  // namespace subtitler
