#ifndef SUBTITLER_GUI_TIMELINE_RULER_H
#define SUBTITLER_GUI_TIMELINE_RULER_H

#include <QTime>
#include <QTimer>
#include <QWidget>
#include <chrono>

#include "subtitler/gui/timeline/indicator.h"
#include "subtitler/gui/timeline/subtitle_interval.h"

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QScrollBar)

namespace subtitler {
namespace gui {
namespace timeline {

/**
 * The ruler widget controls the current indicator position, as well
 * as repositioning the timeline during resizing and zooming.
 * A context menu is available when right clicking.
 */
class Ruler : public QWidget {
    Q_OBJECT
  public:
    explicit Ruler(QWidget* parent, std::chrono::milliseconds duration,
                   const QString& output_srt_file, int zoom_level = 1);
    ~Ruler();

    // Loads internal state from the external output_srt_file.
    // Emits subtitleFileLoaded() signal if successful.
    void LoadSubtitles();

    void setHeaderColor(const QColor& color) { header_bgrnd_ = color; }

    void setDuration(std::chrono::milliseconds duration) {
        resetChildren(duration);
    }

    void setBodyColor(const QColor& color) { body_bgrnd_ = color; }

    void setPlaying(bool playing) { playing_ = playing; }

  signals:
    void changeZoomPosition(int level);
    void changeIndicatorTime(std::chrono::milliseconds ms);
    void userChangedIndicatorTime(std::chrono::milliseconds ms);
    void subtitleIntervalClicked(SubtitleIntervalContainer* container,
                                 SubtitleInterval* interval);
    void changeSubtitleIntervalTime(SubtitleInterval* interval);
    void changeSubtitleIntervalTimeFinished(SubtitleInterval* interval);
    void subtitleFileLoaded(std::size_t num_subtitles);

  public slots:
    void onZoomIn(int level);
    void onZoomOut(int level);
    void onMoveIndicator(std::chrono::milliseconds frame_time);
    void onStepIndicator(std::chrono::milliseconds delta);
    void onAddSubtitleIntervalAfter();
    void onAddSubtitleIntervalBefore();

  protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

  private:
    void setupChildren();
    void resetChildren(std::chrono::milliseconds duration);
    void updateChildren();
    void drawScaleRuler(QPainter* painter, QRectF ruler_rect);
    QString getTickerString(qreal current_pos);
    quint32 msPerInterval();
    qreal lengthPerMs();
    int millisecondsToPosition(const std::chrono::milliseconds& ms);

    // sub controls
    Indicator* indicator_;
    SubtitleIntervalContainer* subtitle_intervals_;
    int zoom_level_;

    // Scroll bar of the parent, since this may manipulate its position.
    QScrollBar* scroll_bar_;

    std::chrono::milliseconds indicator_time_;

    // context menu
    QMenu* context_menu_;
    QAction* add_subtitle_after_;
    QAction* add_subtitle_before_;

    // ruler members
    qreal origin_;
    qreal interval_width_;
    QPoint cursor_pos_;
    QColor body_bgrnd_;
    QColor header_bgrnd_;
    std::chrono::milliseconds duration_;
    qreal rect_width_;
    bool playing_;
};

}  // namespace timeline
}  // namespace gui
}  // namespace subtitler

#endif
