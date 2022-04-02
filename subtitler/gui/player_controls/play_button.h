#ifndef SUBTITLER_GUI_PLAYER_CONTROLS_PLAY_BUTTON_H
#define SUBTITLER_GUI_PLAYER_CONTROLS_PLAY_BUTTON_H

#include <QPushButton>

namespace subtitler {
namespace gui {
namespace player_controls {

/**
 * Play/pause button for the video player. Part of the video controls.
 */
class PlayButton : public QPushButton {
    Q_OBJECT
  public:
    PlayButton(QWidget *parent = Q_NULLPTR);
    ~PlayButton() = default;

    bool is_playing() const { return is_playing_; }

  signals:
    void play();
    void pause();

  public slots:
    void onClick();

  private:
    bool is_playing_;
};

}  // namespace player_controls
}  // namespace gui
}  // namespace subtitler

#endif
