#include "subtitler/gui/video_renderer/opengl_renderer.h"

#include <QPainter>

namespace subtitler {
namespace gui {
namespace video_renderer {

OpenGLRenderer::OpenGLRenderer(QWidget *parent) : QOpenGLWidget(parent) {}

void OpenGLRenderer::presentImage(const QImage &&image) {
    img_ = std::move(image);
    update();
}

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
    // Set the painter to use a smooth scaling algorithm.
    p.setRenderHint(QPainter::SmoothPixmapTransform, 1);

    p.drawImage(QRect(QPoint(0, 0), size()), img_);
}

}  // namespace video_renderer
}  // namespace gui
}  // namespace subtitler
