#include "subtitler/experimental/qt_gui/timeline/subtitle_interval.h"

#include <QFrame>
#include <QLabel>
#include <stdexcept>

#define CUT_MARKER_WIDTH 10
#define CUT_MARKER_HEIGHT 86
#define BODY_HEIGHT 80

using namespace std::chrono_literals;

SubtitleInterval::SubtitleInterval(const SubtitleIntervalArgs& args,
                                   QWidget* parent)
    : QWidget{parent} {
    if (parent == nullptr) {
        throw std::invalid_argument(
            "Parent of SubtitleInvterval cannot be null");
    }

    begin_marker_ = new QLabel(this);
    begin_marker_->setPixmap(QPixmap(":/images/cutleft"));
    begin_marker_->setCursor(Qt::SizeHorCursor);
    begin_marker_->setFixedSize(CUT_MARKER_WIDTH, CUT_MARKER_HEIGHT);
    begin_marker_->move(args.start_x, args.start_y);
    begin_marker_->installEventFilter(parent);
    begin_marker_time_ = args.start_time;

    end_marker_ = new QLabel(this);
    end_marker_->setPixmap(QPixmap(":/images/cutright"));
    end_marker_->setFixedSize(CUT_MARKER_WIDTH, CUT_MARKER_HEIGHT);
    end_marker_->move(args.end_x, args.end_y);
    end_marker_->setCursor(Qt::SizeHorCursor);
    end_marker_->installEventFilter(parent);
    end_marker_time_ = args.end_time;

    rect_box_ = new QFrame(this);
    rect_box_->setObjectName("cutrect");
    updateRect();
}

void SubtitleInterval::MoveBeginMarker(
    const std::chrono::milliseconds& start_time, int x_pos) {
    begin_marker_time_ = start_time;
    begin_marker_->move(x_pos, begin_marker_->y());
    updateRect();
}

void SubtitleInterval::MoveEndMarker(const std::chrono::milliseconds& end_time,
                                     int x_pos) {
    end_marker_time_ = end_time;
    end_marker_->move(x_pos, end_marker_->y());
    updateRect();
}

void SubtitleInterval::updateRect() {
    rect_box_->setGeometry(
        begin_marker_->x() + CUT_MARKER_WIDTH, begin_marker_->y(),
        end_marker_->x() - begin_marker_->x() - CUT_MARKER_WIDTH, BODY_HEIGHT);
    update();
}
