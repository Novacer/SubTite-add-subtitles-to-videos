#include "subtitler/experimental/qt_gui/timeline/subtitle_interval.h"

#include <QFrame>
#include <QLabel>
#include <stdexcept>

#define CUT_MARKER_WIDTH 10
#define CUT_MARKER_HEIGHT 86
#define BODY_HEIGHT 80

using namespace std::chrono_literals;

SubtitleIntervalContainer::SubtitleIntervalContainer(QWidget* parent)
    : QWidget{parent} {}

SubtitleIntervalContainer::~SubtitleIntervalContainer() = default;

void SubtitleIntervalContainer::AddInterval(
    std::unique_ptr<SubtitleInterval> interval) {
    if (!interval) {
        throw std::runtime_error{"Cannot add null interval to container"};
    }
    auto* interval_raw_ptr = interval.get();
    intervals_.push_back(std::move(interval));
    auto [ignore, ok] = marker_to_interval_map_.insert(
        {interval_raw_ptr->GetBeginMarker(), interval_raw_ptr});
    if (!ok) {
        throw std::runtime_error{"Cannot insert duplicate label to container"};
    }
    std::tie(ignore, ok) = marker_to_interval_map_.insert(
        {interval_raw_ptr->GetEndMarker(), interval_raw_ptr});
    if (!ok) {
        throw std::runtime_error{"Cannot insert duplicate label to container"};
    }
    std::tie(ignore, ok) = rect_to_interval_map_.insert({interval_raw_ptr->GetRect(), interval_raw_ptr});
    if (!ok) {
        throw std::runtime_error{"Cannot insert duplicate rect to container"};
    }
}

void SubtitleIntervalContainer::DeleteAll() {
    intervals_.clear();
    marker_to_interval_map_.clear();
    rect_to_interval_map_.clear();
}

SubtitleInterval* SubtitleIntervalContainer::GetIntervalFromMarker(
    QObject* marker) {
    if (auto it = marker_to_interval_map_.find(marker);
        it != marker_to_interval_map_.end()) {
        return it->second;
    }
    return Q_NULLPTR;
}

SubtitleInterval* SubtitleIntervalContainer::GetIntervalFromRect(
    QObject* rect) {
    if (auto it = rect_to_interval_map_.find(rect);
        it != rect_to_interval_map_.end()) {
        return it->second;
    }
    return Q_NULLPTR;
}

SubtitleInterval::SubtitleInterval(const SubtitleIntervalArgs& args,
                                   QWidget* parent) {
    if (parent == Q_NULLPTR) {
        throw std::invalid_argument(
            "Parent of SubtitleInvterval cannot be null");
    }

    begin_marker_ = new QLabel(parent);
    begin_marker_->setPixmap(QPixmap(":/images/cutleft"));
    begin_marker_->setCursor(Qt::SizeHorCursor);
    begin_marker_->setFixedSize(CUT_MARKER_WIDTH, CUT_MARKER_HEIGHT);
    begin_marker_->move(args.start_x, args.start_y);
    begin_marker_->installEventFilter(parent);
    begin_marker_->show();
    begin_marker_time_ = args.start_time;

    end_marker_ = new QLabel(parent);
    end_marker_->setPixmap(QPixmap(":/images/cutright"));
    end_marker_->setFixedSize(CUT_MARKER_WIDTH, CUT_MARKER_HEIGHT);
    end_marker_->move(args.end_x, args.end_y);
    end_marker_->setCursor(Qt::SizeHorCursor);
    end_marker_->installEventFilter(parent);
    end_marker_->show();
    end_marker_time_ = args.end_time;

    rect_box_ = new QLabel(parent);
    rect_box_->setObjectName("cutrect");
    rect_box_->installEventFilter(parent);
    updateRect();
    rect_box_->show();
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

void SubtitleInterval::SetSubtitleText(const QString& subtitle) {
    subtitle_ = subtitle;
    rect_box_->setText(subtitle_);
}

void SubtitleInterval::CleanupWithoutParentAsking() {
    delete begin_marker_;
    begin_marker_ = Q_NULLPTR;
    delete end_marker_;
    end_marker_ = Q_NULLPTR;
    delete rect_box_;
    rect_box_ = Q_NULLPTR;
}

void SubtitleInterval::updateRect() {
    rect_box_->setGeometry(
        begin_marker_->x() + CUT_MARKER_WIDTH, begin_marker_->y(),
        end_marker_->x() - begin_marker_->x() - CUT_MARKER_WIDTH, BODY_HEIGHT);
}
