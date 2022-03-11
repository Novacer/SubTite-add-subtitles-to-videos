#ifndef SUBTITLER_GUI_PLAY_BUTTON_H
#define SUBTITLER_GUI_PLAY_BUTTON_H

#include <QPushButton>

namespace subtitler {
namespace gui {

class PlayButton : public QPushButton {
    Q_OBJECT
  public:
    PlayButton(QWidget *parent = Q_NULLPTR);
    ~PlayButton() = default;

  signals:
    void play();
    void pause();

  public slots:
    void onClick();

  private:
    bool is_playing_;
};

}  // namespace gui
}  // namespace subtitler

#endif
