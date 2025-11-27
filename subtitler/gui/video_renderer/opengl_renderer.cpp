#include "subtitler/gui/video_renderer/opengl_renderer.h"

#include <QPainter>
#include <QtGlobal>
#include <stdexcept>

namespace subtitler {
namespace gui {
namespace video_renderer {

OpenGLRenderer::OpenGLRenderer(QWidget* parent)
    : QOpenGLWidget(parent), img_{} {}

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
// Implementation for older platforms that don't support QVideoFrame::image().
// Reference
// https://github.com/qt/qtmultimedia/blob/5.12.2/src/multimedia/video/qvideoframe.cpp#L1094
void OpenGLRenderer::displayFrame(const QVideoFrame& orig_frame) {
  // Since calling QVideoFrame::map is not const,
  // we remove the const here. I know it's hacky, but the alternative
  // is to copy the entire frame which is worse on performance.
  QVideoFrame& frame = const_cast<QVideoFrame&>(orig_frame);
  if (!frame.isValid() || !frame.map(QAbstractVideoBuffer::ReadOnly)) {
    return;
  }
  QImage::Format imageFormat =
      QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());

  if (imageFormat != QImage::Format_Invalid) {
    img_ = QImage{frame.bits(), frame.width(), frame.height(),
                  frame.bytesPerLine(), imageFormat}
               .copy();
  } else if (frame.pixelFormat() == QVideoFrame::Format_Jpeg) {
    img_.loadFromData(frame.bits(), frame.mappedBytes(), "JPG");
  } else {
    frame.unmap();
    throw std::runtime_error{
        "Cannot handle this format without more help from Qt internals"};
  }

  frame.unmap();
  update();
}

#else
// Implementation for newer platforms that support QVideoFrame::image().
void OpenGLRenderer::displayFrame(const QVideoFrame& orig_frame) {
  img_ = orig_frame.image();
  update();
}

#endif

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
    return QRect((width - widthFromHeight) / 2.0, 0, widthFromHeight, height);
  }
}

void OpenGLRenderer::paintEvent(QPaintEvent*) {
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
