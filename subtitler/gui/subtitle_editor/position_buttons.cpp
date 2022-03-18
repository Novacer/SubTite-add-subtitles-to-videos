#include "subtitler/gui/subtitle_editor/position_buttons.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <stdexcept>

namespace subtitler {
namespace gui {

PositionButtons::PositionButtons(QWidget* parent) : QWidget{parent} {
    radio_top_left_ = new QRadioButton{tr("Top Left"), this};
    radio_top_center_ = new QRadioButton{tr("Top Center"), this};
    radio_top_right_ = new QRadioButton{tr("Top Right"), this};
    radio_middle_left_ = new QRadioButton{tr("Middle Left"), this};
    radio_middle_center_ = new QRadioButton{tr("Middle Center"), this};
    radio_middle_right_ = new QRadioButton{tr("Middle Right"), this};
    radio_bottom_left_ = new QRadioButton{tr("Bottom Left"), this};
    radio_bottom_center_ = new QRadioButton{tr("Bottom Center"), this};
    radio_bottom_right_ = new QRadioButton{tr("Bottom Right"), this};

    connect(radio_top_left_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("top-left"); });
    connect(radio_top_center_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("top-center"); });
    connect(radio_top_right_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("top-right"); });
    connect(radio_middle_left_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("middle-left"); });
    connect(radio_middle_center_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("middle-center"); });
    connect(radio_middle_right_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("middle-right"); });
    connect(radio_bottom_left_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("bottom-left"); });
    connect(radio_bottom_center_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("bottom-center"); });
    connect(radio_bottom_right_, &QRadioButton::clicked, this,
            [this]() { emit positionIdSelected("bottom-right"); });

    radio_bottom_center_->setChecked(true);

    QGridLayout* layout = new QGridLayout{this};
    layout->addWidget(radio_top_left_, 0, 0);
    layout->addWidget(radio_top_center_, 0, 1);
    layout->addWidget(radio_top_right_, 0, 2);
    layout->addWidget(radio_middle_left_, 1, 0);
    layout->addWidget(radio_middle_center_, 1, 1);
    layout->addWidget(radio_middle_right_, 1, 2);
    layout->addWidget(radio_bottom_left_, 2, 0);
    layout->addWidget(radio_bottom_center_, 2, 1);
    layout->addWidget(radio_bottom_right_, 2, 2);
}

void PositionButtons::onPositionChanged(int substation_alpha_id) {
    switch (substation_alpha_id) {
        case 1:
            radio_bottom_left_->setChecked(true);
            break;
        case 2:
            radio_bottom_center_->setChecked(true);
            break;
        case 3:
            radio_bottom_right_->setChecked(true);
            break;
        case 4:
            radio_middle_left_->setChecked(true);
            break;
        case 5:
            radio_middle_center_->setChecked(true);
            break;
        case 6:
            radio_middle_right_->setChecked(true);
            break;
        case 7:
            radio_top_left_->setChecked(true);
            break;
        case 8:
            radio_top_center_->setChecked(true);
            break;
        case 9:
            radio_top_right_->setChecked(true);
            break;
        default:
            throw std::invalid_argument{"Invalid substation_alpha_id"};
    }
}

}  // namespace gui
}  // namespace subtitler
