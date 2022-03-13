#include "subtitler/gui/timeline/subtitle_interval.h"

#include <QDebug>
#include <QFrame>
#include <QLabel>
#include <fstream>
#include <stdexcept>

#include "subtitler/srt/subrip_file.h"

#define CUT_MARKER_WIDTH 10
#define CUT_MARKER_HEIGHT 86
#define BODY_HEIGHT 80

using namespace std::chrono_literals;
namespace fs = std::filesystem;

namespace subtitler {
namespace gui {

SubtitleIntervalContainer::SubtitleIntervalContainer(
    const QString& output_srt_file, QWidget* parent)
    : QWidget{parent},
      output_srt_file_{fs::u8path(output_srt_file.toStdString())} {}

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
    std::tie(ignore, ok) = rect_to_interval_map_.insert(
        {interval_raw_ptr->GetRect(), interval_raw_ptr});
    if (!ok) {
        throw std::runtime_error{"Cannot insert duplicate rect to container"};
    }
}

void SubtitleIntervalContainer::RemoveInterval(SubtitleInterval* interval) {
    if (!interval || intervals_.size() == 0) {
        return;
    }
    // Remove from maps first.
    marker_to_interval_map_.erase(interval->GetBeginMarker());
    marker_to_interval_map_.erase(interval->GetEndMarker());
    rect_to_interval_map_.erase(interval->GetRect());

    // Search for the interval in the vector and delete it.
    for (auto it = intervals_.begin(); it != intervals_.end(); ++it) {
        if (it->get() == interval) {
            it->get()->CleanupWithoutParentAsking();
            intervals_.erase(it);
            interval = Q_NULLPTR;
            return;
        }
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

void SubtitleIntervalContainer::SaveSubripFile() {
    if (output_srt_file_.empty()) {
        return;
    }

    srt::SubRipFile srt_file;
    for (const auto& interval : intervals_) {
        srt_file.AddItem(interval->item_);
    }

    std::ofstream file{output_srt_file_,
                       std::ofstream::out | std::ofstream::trunc};
    if (!file) {
        // TODO: maybe open dialog and warn user rather than using debug?
        qDebug() << "Could not open "
                 << QString::fromStdString(output_srt_file_.u8string());
        return;
    }
    srt_file.ToStream(file);
    qDebug() << "Saved!";
}

bool SubtitleIntervalContainer::LoadSubripFile(qreal interval_width,
                                               quint32 ms_per_interval,
                                               int y_coord) {
    srt::SubRipFile srt_file;
    if (!fs::exists(output_srt_file_) || output_srt_file_.empty()) {
        qDebug() << "No subtitle file to load";
        return false;
    }
    try {
        srt_file.LoadState(output_srt_file_.u8string());
        if (srt_file.NumItems() == 0) {
            qDebug() << "No subtitle items found!";
            return false;
        }
        qDebug() << "Loaded subtitles!";
        DeleteAll();

        const auto& subrip_items = srt_file.GetItems();
        for (const auto& subrip_item : subrip_items) {
            AddInterval(std::make_unique<SubtitleInterval>(
                subrip_item, interval_width, ms_per_interval, y_coord,
                parentWidget()));
        }
    } catch (const std::exception& e) {
        qDebug() << "Failed to load subtitle: " << e.what();
        return false;
    }
    return true;
}

void SubtitleInterval::initializeChildren(QWidget* parent) {
    if (parent == Q_NULLPTR) {
        throw std::invalid_argument(
            "Parent of SubtitleInvterval cannot be null");
    }

    begin_marker_ = new QLabel(parent);
    begin_marker_->setPixmap(QPixmap(":/images/cutleft"));
    begin_marker_->setCursor(Qt::SizeHorCursor);
    begin_marker_->setFixedSize(CUT_MARKER_WIDTH, CUT_MARKER_HEIGHT);
    begin_marker_->installEventFilter(parent);
    begin_marker_->show();

    end_marker_ = new QLabel(parent);
    end_marker_->setPixmap(QPixmap(":/images/cutright"));
    end_marker_->setFixedSize(CUT_MARKER_WIDTH, CUT_MARKER_HEIGHT);
    end_marker_->setCursor(Qt::SizeHorCursor);
    end_marker_->installEventFilter(parent);
    end_marker_->show();

    rect_box_ = new QLabel(parent);
    rect_box_->setObjectName("cutrect");
    rect_box_->installEventFilter(parent);
    rect_box_->show();
}

SubtitleInterval::SubtitleInterval(const SubtitleIntervalArgs& args,
                                   QWidget* parent) {
    initializeChildren(parent);

    begin_marker_->move(args.start_x, args.start_y);
    item_.start(args.start_time);

    end_marker_->move(args.end_x, args.end_y);
    item_.duration(args.end_time - args.start_time);

    updateRect();
}

SubtitleInterval::SubtitleInterval(const srt::SubRipItem& item,
                                   qreal interval_width,
                                   quint32 ms_per_interval, int y_coord,
                                   QWidget* parent)
    : item_{item} {
    initializeChildren(parent);

    int start_x = item.start().count() * interval_width / ms_per_interval;
    begin_marker_->move(start_x, y_coord);

    int end_x = (item.start() + item.duration()).count() * interval_width /
                ms_per_interval;
    end_marker_->move(end_x, y_coord);

    updateRect();

    // Make sure subtitle text state is set properly.
    subtitle_text_ = QString::fromStdString(item.GetPayload());
    SetSubtitleText(subtitle_text_);
}

void SubtitleInterval::MoveBeginMarker(
    const std::chrono::milliseconds& start_time, int x_pos) {
    const auto previous_end_time = item_.start() + item_.duration();
    item_.start(start_time);
    item_.duration(previous_end_time - item_.start());

    begin_marker_->move(x_pos, begin_marker_->y());
    updateRect();
}

void SubtitleInterval::MoveEndMarker(const std::chrono::milliseconds& end_time,
                                     int x_pos) {
    item_.duration(end_time - item_.start());

    end_marker_->move(x_pos, end_marker_->y());
    updateRect();
}

void SubtitleInterval::SetSubtitleText(const QString& subtitle) {
    item_.ClearPayload();
    item_.AppendLine(subtitle.toStdString());
    subtitle_text_ = subtitle;
    rect_box_->setText(subtitle_text_);
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

}  // namespace gui
}  // namespace subtitler
