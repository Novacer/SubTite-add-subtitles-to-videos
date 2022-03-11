#include "subtitler/experimental/qt_gui/player_window.h"

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
#include <QMediaObject>
#include <QMediaService>
#include <QVBoxLayout>
#include <QVideoRendererControl>
#include <QVideoSurfaceFormat>
#include <QVideoWidget>
#include <chrono>
#include <iostream>

#include "subtitler/experimental/qt_gui/play_button.h"
#include "subtitler/experimental/qt_gui/subtitle_editor.h"
#include "subtitler/experimental/qt_gui/timeline/timeline.h"
#include "subtitler/experimental/qt_gui/timeline/timer.h"

class VideoRenderer : public QVideoRendererControl {
  public:
    QAbstractVideoSurface *surface() const override { return m_surface; }

    void setSurface(QAbstractVideoSurface *surface) override {
        m_surface = surface;
    }

    QAbstractVideoSurface *m_surface = Q_NULLPTR;
};

class MediaService : public QMediaService {
  public:
    MediaService(std::unique_ptr<VideoRenderer> vr, QObject *parent = Q_NULLPTR)
        : QMediaService(parent), renderer_(std::move(vr)) {}

    QMediaControl *requestControl(const char *name) override {
        if (qstrcmp(name, QVideoRendererControl_iid) == 0)
            return renderer_.get();

        return Q_NULLPTR;
    }

    void releaseControl(QMediaControl *) override {}

  private:
    std::unique_ptr<VideoRenderer> renderer_ = Q_NULLPTR;
};

class MediaObject : public QMediaObject {
  public:
    MediaObject(std::unique_ptr<MediaService> media_service,
                QObject *parent = Q_NULLPTR)
        : QMediaObject(parent, media_service.get()),
          media_service_{std::move(media_service)} {}

  private:
    std::unique_ptr<MediaService> media_service_;
};

class VideoWidget : public QVideoWidget {
  public:
    explicit VideoWidget(QWidget *parent = Q_NULLPTR) : QVideoWidget(parent) {}
    ~VideoWidget() { delete media_object_; }
    bool setMediaObject(QMediaObject *object) override {
        delete media_object_;
        media_object_ = object;
        return QVideoWidget::setMediaObject(object);
    }

  private:
    QMediaObject *media_object_ = Q_NULLPTR;
};

PlayerWindow::~PlayerWindow() = default;

PlayerWindow::PlayerWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Video Player Demo");
    setMinimumSize(1280, 500);
    QWidget *placeholder = new QWidget{this};
    QVBoxLayout *layout = new QVBoxLayout(placeholder);

    auto video_renderer = std::make_unique<VideoRenderer>();
    video_renderer_ = video_renderer.get();

    VideoWidget *video_widget = new VideoWidget{placeholder};

    MediaObject *media_object = new MediaObject(
        std::make_unique<MediaService>(std::move(video_renderer)),
        video_widget);
    video_widget->setMediaObject(media_object);

    player_ = std::make_unique<QAVPlayer>();

    QString file_name = QFileDialog::getOpenFileName(
        /* parent= */ this,
        /* caption= */ tr("Open Video"),
        /* directory= */ "",
        /* filter= */ tr("Video Files (*.mp4)"));
    if (file_name.isEmpty()) {
        qDebug() << "No video file selected";
        QCoreApplication::exit(1);
    }
    video_file_ = std::make_unique<QFile>(file_name);
    if (!video_file_->open(QFile::ReadOnly)) {
        qDebug() << "Video file could not be opened";
        QCoreApplication::exit(1);
    }
    player_->setSource(file_name, video_file_.get());

    PlayButton *play_button = new PlayButton(placeholder);
    Timer *timer = new Timer{placeholder};

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
    Timeline *timeline = new Timeline{duration, placeholder};

    layout->addWidget(video_widget);
    layout->addWidget(play_button);
    layout->addWidget(timer);
    layout->addWidget(timeline);

    setCentralWidget(placeholder);

    SubtitleEditor *editor = new SubtitleEditor{this};
    editor->setWindowTitle(tr("Subtitle Editor"));
    editor->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, editor);

    // Play/Pause connections
    connect(play_button, &PlayButton::play, player_.get(), &QAVPlayer::play);
    connect(play_button, &PlayButton::pause, player_.get(), &QAVPlayer::pause);
    connect(player_.get(), &QAVPlayer::played, timeline,
            &Timeline::onPlayerPlay);
    connect(player_.get(), &QAVPlayer::paused, timeline,
            &Timeline::onPlayerPause);

    // Connections to synchronize time between ruler, player, and timer.
    connect(timeline, &Timeline::rulerChangedTime, timer,
            &Timer::onTimerChanged);
    connect(timeline, &Timeline::userDraggedRulerChangeTime, this,
            &PlayerWindow::onRulerChangedTime);
    connect(this, &PlayerWindow::playerChangedTime, timeline,
            &Timeline::onPlayerChangedTime);

    audio_output_ = std::make_unique<QAVAudioOutput>();
    // Handle decoded frames.
    connect(player_.get(), &QAVPlayer::audioFrame, this,
            &PlayerWindow::onAudioFrameDecoded);
    connect(player_.get(), &QAVPlayer::videoFrame, this,
            &PlayerWindow::onVideoFrameDecoded);

    // Handle opening/closing subtitle editor
    connect(timeline, &Timeline::openSubtitleEditor, editor,
            &SubtitleEditor::onOpenSubtitle);

    user_seeked_ = false;

    // Init the first frame.
    player_->pause();
}

void PlayerWindow::onRulerChangedTime(std::chrono::milliseconds ms) {
    user_seeked_ = true;
    player_->seek(ms.count());
}

void PlayerWindow::onAudioFrameDecoded(const QAVAudioFrame &audio_frame) {
    if (player_->state() == QAVPlayer::State::PlayingState) {
        audio_output_->play(audio_frame);
    }
}

void PlayerWindow::onVideoFrameDecoded(const QAVVideoFrame &video_frame) {
    if (video_renderer_->m_surface == nullptr) return;
    QVideoFrame videoFrame = video_frame.convertTo(AV_PIX_FMT_RGB32);
    if (!video_renderer_->m_surface->isActive() ||
        video_renderer_->m_surface->surfaceFormat().frameSize() !=
            videoFrame.size()) {
        QVideoSurfaceFormat f(videoFrame.size(), videoFrame.pixelFormat(),
                              videoFrame.handleType());
        video_renderer_->m_surface->start(f);
    }
    if (video_renderer_->m_surface->isActive()) {
        video_renderer_->m_surface->present(videoFrame);
        std::chrono::milliseconds ms{(quint64)(video_frame.pts() * 1000)};
        if (!user_seeked_) {
            // Only emit if the user did not seek.
            // If user seeked, then the player is not changing the time!
            emit this->playerChangedTime(ms);
        } else {
            user_seeked_ = false;
        }
    }
}
