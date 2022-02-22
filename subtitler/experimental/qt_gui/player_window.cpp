#include "subtitler/experimental/qt_gui/player_window.h"

extern "C" {
#include <libavformat/avformat.h>
}

#include <QtAVPlayer/qavaudiooutput.h>
#include <QtAVPlayer/qavplayer.h>
#include <QtAVPlayer/qavvideoframe.h>

#include <QAbstractVideoSurface>
#include <QFileDialog>
#include <QMediaObject>
#include <QMediaService>
#include <QVBoxLayout>
#include <QVideoRendererControl>
#include <QVideoSurfaceFormat>
#include <QVideoWidget>
#include <QWidget>
#include <chrono>
#include <iostream>

#include "subtitler/experimental/qt_gui/play_button.h"
#include "subtitler/experimental/qt_gui/timeline/timeline.h"
#include "subtitler/experimental/qt_gui/timeline/timer.h"

class VideoRenderer : public QVideoRendererControl {
  public:
    QAbstractVideoSurface *surface() const override { return m_surface; }

    void setSurface(QAbstractVideoSurface *surface) override {
        m_surface = surface;
    }

    QAbstractVideoSurface *m_surface = nullptr;
};

class MediaService : public QMediaService {
  public:
    MediaService(VideoRenderer *vr, QObject *parent = nullptr)
        : QMediaService(parent), m_renderer(vr) {}

    QMediaControl *requestControl(const char *name) override {
        if (qstrcmp(name, QVideoRendererControl_iid) == 0) return m_renderer;

        return nullptr;
    }

    void releaseControl(QMediaControl *) override {}

    VideoRenderer *m_renderer = nullptr;
};

class MediaObject : public QMediaObject {
  public:
    explicit MediaObject(VideoRenderer *vr, QObject *parent = nullptr)
        : QMediaObject(parent, new MediaService(vr, parent)) {}
};

class VideoWidget : public QVideoWidget {
  public:
    bool setMediaObject(QMediaObject *object) override {
        return QVideoWidget::setMediaObject(object);
    }
};

PlayerWindow::~PlayerWindow() = default;

PlayerWindow::PlayerWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Video Player Demo");
    setMinimumSize(1000, 500);
    QWidget *placeholder = new QWidget{this};
    QVBoxLayout *layout = new QVBoxLayout(placeholder);

    VideoRenderer *vr = new VideoRenderer;

    VideoWidget *w = new VideoWidget;

    MediaObject *mo = new MediaObject(vr);
    w->setMediaObject(mo);

    player_ = std::make_unique<QAVPlayer>();

    QString file = QFileDialog::getOpenFileName(
        /* parent= */this, 
        /* caption= */ tr("Open Video"),
        /* directory= */ "",
        /* filter= */ tr("Video Files (*.mp4)"));
    player_->setSource(file);

    PlayButton *play_button = new PlayButton(placeholder);
    Timer *timer = new Timer{placeholder};

    // Get video duration
    // TODO: make util class?
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    auto file_stdstr = file.toStdString();
    avformat_open_input(&pFormatCtx, file_stdstr.c_str(), NULL, NULL);
    avformat_find_stream_info(pFormatCtx, NULL);
    auto duration_us = pFormatCtx->duration;
    avformat_close_input(&pFormatCtx);
    avformat_free_context(pFormatCtx);

    std::chrono::milliseconds duration{duration_us / 1000};
    Timeline *timeline = new Timeline{duration, placeholder};

    layout->addWidget(w);
    layout->addWidget(play_button);
    layout->addWidget(timer);
    layout->addWidget(timeline);

    setCentralWidget(placeholder);

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
    connect(player_.get(), &QAVPlayer::audioFrame, player_.get(),
            [=](const QAVAudioFrame &frame) {
                if (this->player_->state() == QAVPlayer::State::PlayingState) {
                    this->audio_output_->play(frame);
                }
            });

    connect(player_.get(), &QAVPlayer::videoFrame, player_.get(),
            [=](const QAVVideoFrame &frame) {
                if (vr->m_surface == nullptr) return;
                QVideoFrame videoFrame = frame.convertTo(AV_PIX_FMT_RGB32);
                if (!vr->m_surface->isActive() ||
                    vr->m_surface->surfaceFormat().frameSize() !=
                        videoFrame.size()) {
                    QVideoSurfaceFormat f(videoFrame.size(),
                                          videoFrame.pixelFormat(),
                                          videoFrame.handleType());
                    vr->m_surface->start(f);
                }
                if (vr->m_surface->isActive()) {
                    vr->m_surface->present(videoFrame);
                    std::chrono::milliseconds ms{(quint64)(frame.pts() * 1000)};
                    emit this->playerChangedTime(ms);
                }
            });

    // Init the first frame.
    player_->pause();
}

void PlayerWindow::onRulerChangedTime(std::chrono::milliseconds ms) {
    player_->seek(ms.count());
}
