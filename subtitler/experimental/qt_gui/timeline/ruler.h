#ifndef SUBTITLER_RULER_H
#define SUBTITLER_RULER_H

#include <QTime>
#include <QTimer>
#include <QWidget>
#include <chrono>

#include "subtitler/experimental/qt_gui/timeline/indicator.h"

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QScrollBar)

class Ruler : public QWidget {
    Q_OBJECT
  public:
    explicit Ruler(QWidget* parent = Q_NULLPTR, quint32 duration = 126,
                   int sliderLevel = 1);
    ~Ruler() = default;

    inline void setHeaderColor(const QColor& color) { header_bgrnd_ = color; }

    inline void setDuration(quint32 duration) { resetChildren(duration); }

    inline void setBodyColor(const QColor& color) { body_bgrnd_ = color; }

    inline int beginTime() const { return begin_marker_time_; }

    inline int endTime() const { return end_marker_time_; }

  signals:
    void changeZoomPosition(int level);
    void changeIndicatorTime(std::chrono::milliseconds ms);

  public slots:
    void onZoomIn(int level);
    void onZoomOut(int level);
    void onMoveIndicator(qreal frameTime);

  protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual bool eventFilter(QObject* watched, QEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

  private:
    void setupChildren();
    void resetChildren(quint32 duration);
    void updateChildren(quint32 prev_seconds_per_interval,
                        qreal prev_interval_width);
    void drawScaleRuler(QPainter* painter, QRectF rulerRect);
    QString getTickerString(qreal tickerNo);
    quint32 secondsPerInterval();
    qreal lengthPerSecond();
    void updateRectBox();

    // sub controls
    Indicator* indicator_;
    QLabel* begin_marker_;
    QLabel* end_marker_;
    QFrame* rect_box_;
    int zoom_level_;

    // Scroll bar of the parent, since this may manipulate its position.
    QScrollBar* scroll_bar_;

    // TODO: convert this to std::chrono::duration
    qreal begin_marker_time_;
    qreal end_marker_time_;
    qreal indicator_time_;

    // context menu
    QMenu* context_menu_;
    QAction* clear_points_;
    QAction* make_current_point_;
    QAction* cut_with_current_pos_;

    // ruler members
    qreal origin_;
    qreal interval_width_;
    QPoint cursor_pos_;
    QColor body_bgrnd_;
    QColor header_bgrnd_;
    // TODO: convert to chrono duration
    quint32 duration_;
    qreal rect_width_;
};

#endif
