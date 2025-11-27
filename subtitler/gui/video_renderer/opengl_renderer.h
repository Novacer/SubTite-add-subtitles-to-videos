#ifndef SUBTITLER_GUI_VIDEO_RENDERER_OPENGL_RENDERER_H
#define SUBTITLER_GUI_VIDEO_RENDERER_OPENGL_RENDERER_H

#include <QOpenGLWidget>
#include <QVideoFrame>

namespace subtitler {
namespace gui {
namespace video_renderer {

/**
 * A video renderer using OpenGL.
 * Suppose you have a decoded QVideoFrame in RGB32 format,
 * then simply call OpenGLRenderer::displayFrame() to display it.
 * Will use hardware acceleration through OpenGL if possible.
 */
class OpenGLRenderer : public QOpenGLWidget {
  Q_OBJECT
 public:
  OpenGLRenderer(QWidget* parent = NULL);
  ~OpenGLRenderer() = default;

  void displayFrame(const QVideoFrame& frame);

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  QImage img_;
  QRect centeredViewport(int width, int height);
};

}  // namespace video_renderer
}  // namespace gui
}  // namespace subtitler

#endif
