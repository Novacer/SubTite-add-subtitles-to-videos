#ifndef SUBTITLER_GUI_PLAYER_WINDOW_H
#define SUBTITLER_GUI_PLAYER_WINDOW_H

#include <QMainWindow>
#include <chrono>
#include <memory>

QT_FORWARD_DECLARE_CLASS(QAVPlayer)
QT_FORWARD_DECLARE_CLASS(QAVAudioOutput)
QT_FORWARD_DECLARE_CLASS(QAVAudioFrame)
QT_FORWARD_DECLARE_CLASS(QAVVideoFrame)
QT_FORWARD_DECLARE_CLASS(QFile)

namespace subtitler::gui {

QT_FORWARD_DECLARE_CLASS(VideoRenderer)

}  // namespace subtitler::gui

namespace subtitler {
namespace gui {

class PlayerWindow : public QMainWindow {
    Q_OBJECT
  public:
    PlayerWindow(QWidget *parent = Q_NULLPTR);
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

}  // namespace gui
}  // namespace subtitler

#endif
