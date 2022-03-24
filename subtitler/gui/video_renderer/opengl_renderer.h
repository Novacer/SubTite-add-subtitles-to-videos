#ifndef SUBTITLER_GUI_VIDEO_RENDERER_OPENGL_RENDERER_H
#define SUBTITLER_GUI_VIDEO_RENDERER_OPENGL_RENDERER_H

#include <QOpenGLWidget>

namespace subtitler {
namespace gui {
namespace video_renderer {

class OpenGLRenderer : public QOpenGLWidget {
    Q_OBJECT
  public:
    OpenGLRenderer(QWidget *parent = NULL);
    ~OpenGLRenderer() = default;

    void presentImage(const QImage &&image);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QImage img_;
    QRect centeredViewport(int width, int height);
};

}  // namespace video_renderer
}  // namespace gui
}  // namespace subtitler

#endif
