#ifndef SUBTITLER_GUI_MAIN_WINDOW_H
#define SUBTITLER_GUI_MAIN_WINDOW_H

#include <QMainWindow>
#include <chrono>
#include <memory>

QT_FORWARD_DECLARE_CLASS(QAVPlayer)
QT_FORWARD_DECLARE_CLASS(QAVAudioOutput)
QT_FORWARD_DECLARE_CLASS(QAVAudioFrame)
QT_FORWARD_DECLARE_CLASS(QAVVideoFrame)
QT_FORWARD_DECLARE_CLASS(QFile)

namespace subtitler::gui {
namespace subtitle_editor {

QT_FORWARD_DECLARE_CLASS(SubtitleEditor)

}  // namespace subtitle_editor

namespace video_renderer {

QT_FORWARD_DECLARE_CLASS(OpenGLRenderer)

}  // namespace video_renderer
}  // namespace subtitler::gui

namespace subtitler {
namespace gui {

/**
 * Main window for the video editor gui.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow();

  signals:
    void playerChangedTime(std::chrono::milliseconds ms);

  public slots:
    void onRulerChangedTime(std::chrono::milliseconds ms);
    void onAudioFrameDecoded(const QAVAudioFrame &audio_frame);
    void onVideoFrameDecoded(const QAVVideoFrame &video_frame);
    void onSubtitleFileChanged(std::size_t num_loaded);
    void onExport(bool checked);

  private:
    std::unique_ptr<QAVPlayer> player_;
    std::unique_ptr<QAVAudioOutput> audio_output_;
    std::unique_ptr<QFile> video_file_;
    video_renderer::OpenGLRenderer *video_renderer_;
    subtitle_editor::SubtitleEditor *editor_;
    // Flag var which is set when user moves the ruler,
    // and reset once the next video frame is decoded.
    bool user_seeked_;
    QString subtitle_file_;
};

}  // namespace gui
}  // namespace subtitler

#endif
