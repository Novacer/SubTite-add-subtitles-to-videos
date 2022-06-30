#ifndef SUBTITLER_GUI_TIMELINE_SUBTITLE_INTERVAL_H
#define SUBTITLER_GUI_TIMELINE_SUBTITLE_INTERVAL_H

#include <QString>
#include <QWidget>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include "subtitler/srt/subrip_item.h"

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QFrame)

namespace subtitler::gui::timeline {

QT_FORWARD_DECLARE_CLASS(SubtitleInterval)

}  // namespace subtitler::gui::timeline

namespace subtitler {
namespace gui {
namespace timeline {

/**
 * Widget containing all of the subtitle intervals.
 * Example usage:
 * auto* container = new SubtitleIntervalContainer(this);
 * SubtitleIntervalArgs args{ ... };
 * container->AddInterval(std::make_unique<SubtitleInverval>(args, this));
 */
class SubtitleIntervalContainer : public QWidget {
    Q_OBJECT
  public:
    SubtitleIntervalContainer(const QString& output_srt_file,
                              QWidget* parent = Q_NULLPTR);
    ~SubtitleIntervalContainer();

    // Unique_ptr required to memory manage SubtitleInterval since it is
    // not a widget. See documentation below on why that is.
    void AddInterval(std::unique_ptr<SubtitleInterval> interval);

    void RemoveInterval(SubtitleInterval* interval);

    void DeleteAll();

    SubtitleInterval* GetIntervalFromMarker(QObject* marker);
    SubtitleInterval* GetIntervalFromRect(QObject* rect);

    std::vector<std::unique_ptr<SubtitleInterval>>& intervals() {
        return intervals_;
    };

    // Params needed to calculate position of the intervals.
    // Reference examples in ruler.cpp to see how the fields are set.
    std::pair<bool, std::size_t> LoadSubripFile(qreal interval_width,
                                                quint32 ms_per_interval,
                                                int y_coord);
    void ChangeSubripFile(const QString& new_subrip_file);

  public slots:
    void SaveSubripFile();

  private:
    std::vector<std::unique_ptr<SubtitleInterval>> intervals_;
    std::unordered_map<QObject*, SubtitleInterval*> marker_to_interval_map_;
    std::unordered_map<QObject*, SubtitleInterval*> rect_to_interval_map_;
    std::filesystem::path output_srt_file_;
};

/**
 * Struct containing arguments to pass to SubtitleInterval's ctor.
 */
struct SubtitleIntervalArgs {
    int start_x = 0;
    int start_y = 0;
    int end_x = 0;
    int end_y = 0;
    std::chrono::milliseconds start_time;
    std::chrono::milliseconds end_time;
};

/**
 * A Plain Old Object containing various labels and rectangles
 * representing an interval of time for which a subtitle should be displayed.
 *
 * NOTE: Despite having a QWidget parent in the ctor, this is NOT a widget.
 * This means you must manually manage the memory of this class!
 * It is recommended to be used in conjunction with SubtitleIntervalContainer.
 *
 * It is not a widget because we want to make the members begin_marker_ etc.
 * direct children of the widget containing SubtitlerIntervalContainer.
 * This allows a "flat" hierarchy which avoids a lot of geometry issues.
 */
class SubtitleInterval {
  public:
    SubtitleInterval(const SubtitleIntervalArgs& args, QWidget* parent);
    SubtitleInterval(const std::shared_ptr<srt::SubRipItem>& item,
                     qreal interval_width, quint32 ms_per_interval, int y_coord,
                     QWidget* parent);
    // TODO: if parent is null, should this cleanup the children?
    ~SubtitleInterval() = default;

    void MoveBeginMarker(const std::chrono::milliseconds& start_time,
                         int x_pos);
    void MoveEndMarker(const std::chrono::milliseconds& end_time, int x_pos);

    QString GetSubtitleText() const { return subtitle_text_; }
    void SetSubtitleText(const QString& subtitle);

    int GetSubtitlePosition() const {
        return item_->substation_alpha_position();
    }
    void SetSubtitlePosition(const std::string& position_id);

    std::chrono::milliseconds GetBeginTime() const { return item_->start(); }
    std::chrono::milliseconds GetEndTime() const {
        return item_->start() + item_->duration();
    }

    QLabel* GetBeginMarker() const { return begin_marker_; }
    QLabel* GetEndMarker() const { return end_marker_; }

    QLabel* GetRect() const { return rect_box_; }

    // Cleans up child widgets. Should only be used if SubtitleInterval
    // is going to be destroyed from a context where it is NOT the parent
    // widget destroying it's children. (Parent refers to the parent of
    // SubtitleIntervalContainer as well).
    void CleanupWithoutParentAsking();

  private:
    QLabel* begin_marker_;
    QLabel* end_marker_;
    QLabel* rect_box_;

    std::shared_ptr<srt::SubRipItem> item_;
    // Payload of SubRipItem may contain  additional formatting, so we also
    // separately store the subtitle_text before any formatting.
    QString subtitle_text_;

    void updateRect();
    void initializeChildren(QWidget* parent);

    friend class SubtitleIntervalContainer;
};

}  // namespace timeline
}  // namespace gui
}  // namespace subtitler

#endif
