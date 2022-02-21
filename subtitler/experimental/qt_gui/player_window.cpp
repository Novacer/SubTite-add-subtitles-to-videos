#include "subtitler/experimental/qt_gui/player_window.h"

#include <QtAVPlayer/qavaudiooutput.h>
#include <QtAVPlayer/qavplayer.h>
#include <QtAVPlayer/qavvideoframe.h>

#include <QAbstractVideoSurface>
#include <QMediaObject>
#include <QMediaService>
#include <QVBoxLayout>
#include <QVideoRendererControl>
#include <QVideoSurfaceFormat>
#include <QVideoWidget>
#include <QWidget>
#include <chrono>

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
    setMinimumSize(1000, 1000);
    QWidget *placeholder = new QWidget{this};
    QVBoxLayout *layout = new QVBoxLayout(placeholder);

    VideoRenderer *vr = new VideoRenderer;

    VideoWidget *w = new VideoWidget;

    MediaObject *mo = new MediaObject(vr);
    w->setMediaObject(mo);

    player_ = std::make_unique<QAVPlayer>();

    // TODO: build a component to select files dynamically.
    QString file = QLatin1String(
        "D:\\Lecture Videos\\CLAS_201 Videos\\Week "
        "1ii - Introduction.mp4");
    player_->setSource(file);

    PlayButton *play_button = new PlayButton(placeholder);
    Timer *timer = new Timer{placeholder};
    Timeline *timeline = new Timeline{placeholder};

    layout->addWidget(w);
    layout->addWidget(play_button);
    layout->addWidget(timer);
    layout->addWidget(timeline);

    setCentralWidget(placeholder);

    // Play/Pause connections
    connect(play_button, &PlayButton::play, player_.get(), &QAVPlayer::play);
    connect(play_button, &PlayButton::pause, player_.get(), &QAVPlayer::pause);
    connect(player_.get(), &QAVPlayer::played, timeline, &Timeline::onPlayerPlay);
    connect(player_.get(), &QAVPlayer::paused, timeline, &Timeline::onPlayerPause);

    // Connections to synchronize time between ruler, player, and timer.
    connect(timeline, &Timeline::rulerChangedTime, timer,
            &Timer::onTimerChanged);
    connect(timeline, &Timeline::userDraggedRulerChangeTime, this,
            &PlayerWindow::onRulerChangedTime);
    connect(this, &PlayerWindow::playerChangedTime, timeline,
            &Timeline::onPlayerChangedTime);

    // TODO: unique_ptr?
    QAVAudioOutput *audioOutput = new QAVAudioOutput;
    connect(player_.get(), &QAVPlayer::audioFrame, player_.get(),
            [=](const QAVAudioFrame &frame) {
                if (this->player_->state() == QAVPlayer::State::PlayingState) {
                    audioOutput->play(frame);
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
    player_->seek(0);
}

void PlayerWindow::onRulerChangedTime(std::chrono::milliseconds ms) {
    player_->seek(ms.count());
}
