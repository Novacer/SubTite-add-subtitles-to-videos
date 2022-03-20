#include "subtitler/gui/player_controls/play_button.h"

namespace subtitler {
namespace gui {

PlayButton::PlayButton(QWidget *parent)
    : QPushButton{parent}, is_playing_{false} {
    setIcon(QIcon(":/images/play"));
    connect(this, &QPushButton::clicked, this, &PlayButton::onClick);
}

void PlayButton::onClick() {
    is_playing_ = !is_playing_;
    if (is_playing_) {
        setIcon(QIcon(":/images/pause"));
        emit play();
    } else {
        setIcon(QIcon(":/images/play"));
        emit pause();
    }
}

}  // namespace gui
}  // namespace subtitler
