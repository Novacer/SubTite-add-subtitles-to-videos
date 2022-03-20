#include "subtitler/gui/player_controls/step_button.h"

using namespace std::chrono_literals;

namespace subtitler {
namespace gui {

StepButton::StepButton(const std::chrono::milliseconds& step_amount,
                       QWidget* parent)
    : QPushButton{parent}, step_amount{step_amount} {
    connect(this, &StepButton::clicked, this, &StepButton::onClicked);
}

void StepButton::onClicked() { emit stepDelta(step_amount); }

StepForwardsButton::StepForwardsButton(QWidget* parent)
    : StepButton{5s, parent} {
    setIcon(QIcon(":/images/stepForward"));
}

StepBackwardsButton::StepBackwardsButton(QWidget* parent)
    : StepButton{-5s, parent} {
    setIcon(QIcon(":/images/stepBackward"));
}

}  // namespace gui
}  // namespace subtitler
