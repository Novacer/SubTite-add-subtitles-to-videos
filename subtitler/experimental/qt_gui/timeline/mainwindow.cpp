#include "subtitler/experimental/qt_gui/timeline/mainwindow.h"

#include <QVBoxLayout>
#include <QWidget>

#include "subtitler/experimental/qt_gui/timeline/timeline.h"
#include "subtitler/experimental/qt_gui/timeline/timer.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Video Timeline Demo");
    setMinimumSize(1000, 150);
    setWindowIcon(QIcon(":/images/logo"));

    QWidget *placeholder = new QWidget{this};
    QVBoxLayout *layout = new QVBoxLayout(placeholder);

    Timer *timer = new Timer{placeholder};
    Timeline *timeline = new Timeline{placeholder};

    timer->setAlignment(Qt::AlignCenter);
    layout->addWidget(timer);
    layout->addWidget(timeline);

    connect(timeline, &Timeline::rulerChangedTime, timer,
            &Timer::onTimerChanged);

    setCentralWidget(placeholder);
}
