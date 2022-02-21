#include "subtitler/experimental/qt_gui/timeline/mainwindow.h"

#include <QDebug>
#include <QScrollBar>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent /* = Q_NULLPTR */)
    : QScrollArea(parent) {
    setWindowIcon(QIcon(":/images/logo"));
    resize(1000, 150);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    quint32 duration = 600;
    zoomer_ = new Zoomer(this, duration);
    zoomer_->setMinimumWidth(300);
    addScrollBarWidget(zoomer_, Qt::AlignLeft);

    ruler_ = new Ruler(this, duration, zoomer_->max_zoom_level());
    setWidget(ruler_);

    connect(zoomer_, &Zoomer::zoomIn, ruler_, &Ruler::onZoomIn);
    connect(zoomer_, &Zoomer::zoomOut, ruler_, &Ruler::onZoomOut);
    connect(ruler_, &Ruler::changeSliderPosition, zoomer_,
            &Zoomer::onSliderChanged);
}
