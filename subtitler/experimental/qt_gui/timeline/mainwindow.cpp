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
    mZoomer = new Zoomer(this, duration);
    mZoomer->setMinimumWidth(300);
    addScrollBarWidget(mZoomer, Qt::AlignLeft);

    mRuler = new Ruler(this, duration, mZoomer->max_zoom_level());
    setWidget(mRuler);

    connect(mZoomer, &Zoomer::zoomerIn, mRuler, &Ruler::onZoomerIn);
    connect(mZoomer, &Zoomer::zoomerOut, mRuler, &Ruler::onZoomerOut);
    connect(mRuler, &Ruler::changeSliderPosition, mZoomer,
            &Zoomer::onChangeSliderPosition);
}
