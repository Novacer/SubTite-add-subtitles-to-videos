#ifndef SUBTITLER_GUI_PLAYER_CONTROLS_STEP_BUTTON_H
#define SUBTITLER_GUI_PLAYER_CONTROLS_STEP_BUTTON_H

#include <QPushButton>
#include <chrono>

namespace subtitler {
namespace gui {

/**
 * Generalized interface for a button moving player forwards/backwards.
 * See below for more specializations.
 */
class StepButton : public QPushButton {
    Q_OBJECT
  public:
    StepButton(const std::chrono::milliseconds &step_amount,
               QWidget *parent = Q_NULLPTR);
    virtual ~StepButton() = default;

  signals:
    void stepDelta(std::chrono::milliseconds delta);

  protected slots:
    virtual void onClicked();

  private:
    std::chrono::milliseconds step_amount;
};

/**
 * Button for stepping player fowards by 5 seconds.
 */
class StepForwardsButton : public StepButton {
    Q_OBJECT
  public:
    StepForwardsButton(QWidget *parent = Q_NULLPTR);
    ~StepForwardsButton() = default;
};

/**
 * Button for stepping player backwards by 5 seconds.
 */
class StepBackwardsButton : public StepButton {
    Q_OBJECT
  public:
    StepBackwardsButton(QWidget *parent = Q_NULLPTR);
    ~StepBackwardsButton() = default;
};

}  // namespace gui
}  // namespace subtitler

#endif
