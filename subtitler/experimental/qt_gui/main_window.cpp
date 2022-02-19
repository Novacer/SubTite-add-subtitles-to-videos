#include "subtitler/experimental/qt_gui/main_window.h"

#include <QtWidgets/QAction>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("QT Demo");
    setMinimumSize(800, 450);

    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    QMenu *menu = menuBar->addMenu("&File");

    QAction *action = new QAction("&New", this);
    connect(action, &QAction::triggered, this, &MainWindow::OnFileNew);
    menu->addAction(action);

    action = new QAction("&Open", this);
    connect(action, &QAction::triggered, this, &MainWindow::OnFileOpen);
    menu->addAction(action);

    action = new QAction("&Save", this);
    connect(action, &QAction::triggered, this, &MainWindow::OnFileSave);
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction("&Exit", this);
    connect(action, &QAction::triggered, this, &MainWindow::close);
    menu->addAction(action);

    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
}

void MainWindow::OnFileNew() { statusBar()->showMessage("File -> New"); }

void MainWindow::OnFileOpen() { statusBar()->showMessage("File -> Open"); }

void MainWindow::OnFileSave() { statusBar()->showMessage("File -> Save"); }
