#ifndef SUBTITLER_PLAYER_WINDOW_H
#define SUBTITLER_PLAYER_WINDOW_H

#include <QMainWindow>
#include <chrono>
#include <memory>

class QAVPlayer;
class QAVAudioOutput;
class QAVAudioFrame;
class QAVVideoFrame;
class VideoRenderer;
class QFile;

class PlayerWindow : public QMainWindow {
    Q_OBJECT

  public:
    PlayerWindow(QWidget *parent = nullptr);
    ~PlayerWindow();

  signals:
    void playerChangedTime(std::chrono::milliseconds ms);

  public slots:
    void onRulerChangedTime(std::chrono::milliseconds ms);
    void onAudioFrameDecoded(const QAVAudioFrame &audio_frame);
    void onVideoFrameDecoded(const QAVVideoFrame &video_frame);

  private:
    std::unique_ptr<QAVPlayer> player_;
    std::unique_ptr<QAVAudioOutput> audio_output_;
    std::unique_ptr<QFile> video_file_;
    VideoRenderer *video_renderer_;
    // Flag var which is set when user moves the ruler,
    // and reset once the next video frame is decoded.
    bool user_seeked_;
};

#endif
