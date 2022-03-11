#ifndef SUBTITLER_SUBTITLE_INTERVAL_H
#define SUBTITLER_SUBTITLE_INTERVAL_H

#include <QWidget>
#include <chrono>
#include <memory>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QFrame)
QT_FORWARD_DECLARE_CLASS(SubtitleInterval)

// TODO docs.
class SubtitleIntervalContainer : public QWidget {
    Q_OBJECT
  public:
    SubtitleIntervalContainer(QWidget* parent = Q_NULLPTR);
    ~SubtitleIntervalContainer();

    // Unique_ptr required to memory manage SubtitleInterval since it is
    // not a widget. See documentation below on why that is.
    void AddInterval(std::unique_ptr<SubtitleInterval> interval);
    void DeleteAll();

    SubtitleInterval* GetIntervalFromMarker(QObject* marker);

    std::vector<std::unique_ptr<SubtitleInterval>>& intervals() {
        return intervals_;
    };

  private:
    std::vector<std::unique_ptr<SubtitleInterval>> intervals_;
    std::unordered_map<QObject*, SubtitleInterval*> marker_to_interval_map;
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
    // TODO: if parent is null, should this cleanup the children?
    ~SubtitleInterval() = default;

    void MoveBeginMarker(const std::chrono::milliseconds& start_time,
                         int x_pos);
    void MoveEndMarker(const std::chrono::milliseconds& end_time, int x_pos);

    std::chrono::milliseconds GetBeginTime() const {
        return begin_marker_time_;
    }
    std::chrono::milliseconds GetEndTime() const { return end_marker_time_; }
    QLabel* GetBeginMarker() const { return begin_marker_; }
    QLabel* GetEndMarker() const { return end_marker_; }

  private:
    QLabel* begin_marker_;
    std::chrono::milliseconds begin_marker_time_;

    QLabel* end_marker_;
    std::chrono::milliseconds end_marker_time_;

    QFrame* rect_box_;

    void updateRect();
};

#endif
