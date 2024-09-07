#include "subtitler/gui/video_renderer/opengl_renderer.h"

#include <QPainter>
#include <stdexcept>

namespace subtitler {
namespace gui {
namespace video_renderer {

OpenGLRenderer::OpenGLRenderer(QWidget *parent)
    : QOpenGLWidget(parent), img_{} {}

void OpenGLRenderer::displayFrame(const QVideoFrame &orig_frame) {
    img_ = orig_frame.image();
    update();
}

/**
 * Given a rectangle with of width and height, return a smaller rectangle which
 * has the same aspect ratio as the image. The returned rectangle will be
 * centered within the outside one.
 *
 * @param width the width of the outside rectangle.
 * @param height the height of the outside rectangle.
 * @return QRect the centered rectangle with the same aspect ratio as the image.
 */
QRect OpenGLRenderer::centeredViewport(int width, int height) {
    qreal aspectRatio = 1.0;
    if (!img_.isNull()) {
        aspectRatio = (qreal)img_.width() / (qreal)img_.height();
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

void OpenGLRenderer::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setViewport(centeredViewport(width(), height()));
    // Disable scaling (mostly to help with performance).
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);
    p.drawImage(QRect(QPoint(0, 0), size()), img_);
}

}  // namespace video_renderer
}  // namespace gui
}  // namespace subtitler
