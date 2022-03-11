#ifndef SUBTITLER_RULER_H
#define SUBTITLER_RULER_H

#include <QTime>
#include <QTimer>
#include <QWidget>
#include <chrono>

#include "subtitler/gui/timeline/indicator.h"
#include "subtitler/gui/timeline/subtitle_interval.h"

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QScrollBar)

class Ruler : public QWidget {
    Q_OBJECT
  public:
    explicit Ruler(QWidget* parent, std::chrono::milliseconds duration,
                   int zoom_level = 1);
    ~Ruler();

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
    void subtitleIntervalClicked(SubtitleInterval* interval);

  public slots:
    void onZoomIn(int level);
    void onZoomOut(int level);
    void onMoveIndicator(std::chrono::milliseconds frame_time);
    void onAddSubtitleInterval();

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
    QAction* add_subtitle_interval_;

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

#endif
