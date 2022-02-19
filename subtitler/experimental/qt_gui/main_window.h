#ifndef SUBTITLER_EXPERIMENTAL_QT_GUI_MAIN_WINDOW_H
#define SUBTITLER_EXPERIMENTAL_QT_GUI_MAIN_WINDOW_H

#include <QtWidgets/QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    MainWindow(QWidget *parent = nullptr);

  private slots:
    void OnFileNew();
    void OnFileOpen();
    void OnFileSave();
};

#endif
