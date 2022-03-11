#ifndef SUBTITLER_SUBTITLE_INTERVAL_H
#define SUBTITLER_SUBTITLE_INTERVAL_H

#include <QWidget>
#include <chrono>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QFrame)

struct SubtitleIntervalArgs {
    int start_x;
    int start_y;
    int end_x;
    int end_y;
    std::chrono::milliseconds start_time;
    std::chrono::milliseconds end_time;
};

class SubtitleInterval : public QWidget {
    Q_OBJECT
  public:
    SubtitleInterval(const SubtitleIntervalArgs& args, QWidget* parent);
    ~SubtitleInterval() = default;

    void MoveBeginMarker(const std::chrono::milliseconds& start_time,
                         int x_pos);
    void MoveEndMarker(const std::chrono::milliseconds& end_time,
                       int x_pos);

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
