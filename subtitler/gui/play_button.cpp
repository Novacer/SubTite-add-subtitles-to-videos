#include "subtitler/gui/play_button.h"

namespace subtitler {
namespace gui {
namespace {

const char *PLAY_MSG = "play";
const char *PAUSE_MSG = "pause";

}  // namespace

PlayButton::PlayButton(QWidget *parent)
    : QPushButton{PLAY_MSG, parent}, is_playing_{false} {
    connect(this, &QPushButton::clicked, this, &PlayButton::onClick);
}

void PlayButton::onClick() {
    is_playing_ = !is_playing_;
    if (is_playing_) {
        setText(PAUSE_MSG);
        emit play();
    } else {
        setText(PLAY_MSG);
        emit pause();
    }
}

}  // namespace gui
}  // namespace subtitler
