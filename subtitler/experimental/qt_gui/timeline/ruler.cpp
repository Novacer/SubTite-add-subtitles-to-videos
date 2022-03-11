#include "subtitler/experimental/qt_gui/timeline/ruler.h"

#include <QAbstractScrollArea>
#include <QAction>
#include <QContextMenuEvent>
#include <QCursor>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "subtitler/util/duration_format.h"

#define HEADER_HEIGHT 40
#define BODY_HEIGHT 80
#define START_END_PADDING 60
#define CUT_MARKER_WIDTH 10
#define TIME_LABEL_OFFSET 10

using namespace std::chrono_literals;

Ruler::Ruler(QWidget* parent, std::chrono::milliseconds duration,
             int zoom_level)
    : QWidget{parent},
      zoom_level_{zoom_level},
      scroll_bar_{Q_NULLPTR},
      origin_{10.0},
      interval_width_{130.0},
      body_bgrnd_{37, 38, 39},
      header_bgrnd_{32, 32, 32},
      duration_{duration},
      rect_width_{interval_width_ * duration_.count() / msPerInterval()},
      playing_{false} {
    subtitle_intervals_ = new SubtitleIntervalContainer(this);
    if (!subtitle_intervals_) {
        throw std::runtime_error{"Unable allocate subtitle container"};
    }

    setAttribute(Qt::WA_OpaquePaintEvent);

    if (auto scroll_area = dynamic_cast<QAbstractScrollArea*>(parent)) {
        scroll_bar_ = scroll_area->horizontalScrollBar();
    }

    setupChildren();

    context_menu_ = new QMenu(this);
    add_subtitle_interval_ = new QAction(tr("Add Subtitle at Indicator"), this);

    connect(add_subtitle_interval_, &QAction::triggered, this,
            &Ruler::onAddSubtitleInterval);

    resize(rect_width_ + START_END_PADDING, 120);
}

Ruler::~Ruler() = default;

void Ruler::setupChildren() {
    indicator_ = new Indicator(this);
    indicator_->installEventFilter(this);
    indicator_time_ = 0ms;
}

int Ruler::millisecondsToPosition(const std::chrono::milliseconds& ms) {
    return interval_width_ * ms.count() / msPerInterval();
}

void Ruler::resetChildren(std::chrono::milliseconds duration) {
    duration_ = duration;

    rect_width_ = duration_.count() * interval_width_ / msPerInterval();
    indicator_->move(0, 0);

    subtitle_intervals_->DeleteAll();

    resize(rect_width_ + START_END_PADDING, HEADER_HEIGHT + BODY_HEIGHT);
}

void Ruler::onMoveIndicator(std::chrono::milliseconds frame_time) {
    if (frame_time > duration_) {
        return;
    }
    indicator_->move(frame_time.count() * lengthPerMs(), indicator_->y());
    indicator_time_ = frame_time;
    emit changeIndicatorTime(indicator_time_);
}

// update children when the ruler scaled up or down
void Ruler::updateChildren() {
    rect_width_ = interval_width_ * duration_.count() / msPerInterval();

    for (auto& interval : subtitle_intervals_->intervals()) {
        auto begin_time = interval->GetBeginTime();
        interval->MoveBeginMarker(begin_time,
                                  millisecondsToPosition(begin_time));
        auto end_time = interval->GetEndTime();
        interval->MoveEndMarker(end_time, millisecondsToPosition(end_time));
    }
    indicator_->move(
        indicator_time_.count() * interval_width_ / msPerInterval(),
        indicator_->y());

    // When zooming, ensure that start point of the interval is preserved.
    // Theoretically possible to "zoom on mouse point". Will leave as TODO.
    qreal prev_time_ms = 0;
    if (scroll_bar_) {
        qreal prev_ms_per_scroll_tick =
            (qreal)duration_.count() /
            (scroll_bar_->maximum() + scroll_bar_->pageStep());
        prev_time_ms = scroll_bar_->value() * prev_ms_per_scroll_tick;
    }

    // Perform resize.
    int new_width = rect_width_ + START_END_PADDING;
    resize(new_width, HEADER_HEIGHT + BODY_HEIGHT);
    update();

    // Try to keep start point at the same timestamp as before.
    if (scroll_bar_ && duration_ != 0ms) {
        qreal new_scroll_tick_per_ms =
            (scroll_bar_->maximum() + scroll_bar_->pageStep()) /
            (qreal)duration_.count();
        int new_start = prev_time_ms * new_scroll_tick_per_ms;
        scroll_bar_->setValue(new_start);
    }
}

bool Ruler::eventFilter(QObject* watched, QEvent* event) {
    if (watched == indicator_ ||
        subtitle_intervals_->GetIntervalFromMarker(watched) != Q_NULLPTR) {
        static QPoint lastPnt;
        static bool isHover = false;
        if (event->type() == QEvent::MouseButtonPress) {
            QLabel* control = dynamic_cast<QLabel*>(watched);
            QMouseEvent* e = static_cast<QMouseEvent*>(event);
            if (control->rect().contains(e->pos()) &&
                (e->button() == Qt::LeftButton)) {
                lastPnt = e->pos();
                isHover = true;
            }
        } else if (event->type() == QEvent::MouseMove && isHover) {
            QMouseEvent* e = dynamic_cast<QMouseEvent*>(event);
            int dx = e->pos().x() - lastPnt.x();

            // For now, disable moving indicator while video is playing.
            if (watched == indicator_ && !playing_) {
                if (indicator_->x() + dx <= rect_width_ - CUT_MARKER_WIDTH &&
                    indicator_->x() + dx >= 0) {
                    qreal new_indicator_time =
                        (indicator_->x() + dx) / lengthPerMs();
                    indicator_time_ =
                        std::chrono::milliseconds((quint64)new_indicator_time);
                    indicator_->move(indicator_->x() + dx, indicator_->y());
                    emit changeIndicatorTime(indicator_time_);
                }
            }
            SubtitleInterval* interval =
                subtitle_intervals_->GetIntervalFromMarker(watched);
            if (!interval) {
                return false;
            }

            if (auto begin_marker = interval->GetBeginMarker();
                watched == begin_marker) {
                if (begin_marker->x() + dx + CUT_MARKER_WIDTH <= rect_width_ &&
                    begin_marker->x() + dx >= 0 &&
                    begin_marker->x() + dx + CUT_MARKER_WIDTH <=
                        interval->GetEndMarker()->x()) {
                    auto new_begin_marker_time = std::chrono::milliseconds{
                        (quint64)((begin_marker->x() + dx) / lengthPerMs())};
                    interval->MoveBeginMarker(
                        new_begin_marker_time,
                        millisecondsToPosition(new_begin_marker_time));
                }
            }
            if (auto end_marker = interval->GetEndMarker();
                watched == end_marker) {
                if (end_marker->x() + dx <= rect_width_ + CUT_MARKER_WIDTH &&
                    end_marker->x() + dx >= 0 &&
                    end_marker->x() + dx - CUT_MARKER_WIDTH >=
                        interval->GetBeginMarker()->x()) {
                    auto new_end_marker_time = std::chrono::milliseconds{
                        (quint64)((end_marker->x() + dx) / lengthPerMs())};
                    interval->MoveEndMarker(
                        new_end_marker_time,
                        millisecondsToPosition(new_end_marker_time));
                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease && isHover) {
            isHover = false;
            if (watched == indicator_ && !playing_) {
                // Only emit this when user releases mouse to prevent
                // spamming video player with seeks and overloading the decoder.
                emit userChangedIndicatorTime(indicator_time_);
            }
        }
    } else if (auto* interval =
                   subtitle_intervals_->GetIntervalFromRect(watched);
               interval != Q_NULLPTR) {
        if (event->type() == QEvent::MouseButtonRelease) {
            emit subtitleIntervalClicked(interval);
        }
    }

    return false;
}

void Ruler::contextMenuEvent(QContextMenuEvent* event) {
    context_menu_->addAction(add_subtitle_interval_);
    context_menu_->exec(QCursor::pos());
    event->accept();
}

void Ruler::wheelEvent(QWheelEvent* event) {
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
        if (numDegrees.y() > 0) {
            emit changeZoomPosition(zoom_level_ - 1);
        } else if (numDegrees.y() < 0) {
            emit changeZoomPosition(zoom_level_ + 1);
        }
    }
    event->accept();
}

void Ruler::mousePressEvent(QMouseEvent* event) {}

void Ruler::mouseReleaseEvent(QMouseEvent* event) {}

void Ruler::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.setRenderHints(QPainter::TextAntialiasing |
                           QPainter::HighQualityAntialiasing);

    QRectF rulerRect = this->rect();
    // paint header background color
    painter.fillRect(
        QRect(rulerRect.left(), rulerRect.top(), this->width(), HEADER_HEIGHT),
        header_bgrnd_);
    // paint body background color
    painter.fillRect(QRect(rulerRect.left(), rulerRect.top() + HEADER_HEIGHT,
                           this->width(), rulerRect.height() - HEADER_HEIGHT),
                     body_bgrnd_);

    if (duration_ > 0ms) {
        // draw tickers and time labels
        drawScaleRuler(&painter, rulerRect);
    }
}

quint32 Ruler::msPerInterval() {
    if (zoom_level_ <= 1) {
        return 1000;
    }
    // Time increases linearly as slider level increases.
    return (zoom_level_ - 1) * 1000 * 10;
}

qreal Ruler::lengthPerMs() { return interval_width_ / msPerInterval(); }

QString Ruler::getTickerString(qreal current_pos) {
    qreal pos = current_pos - origin_;
    int interval_num = pos / interval_width_;
    if (interval_num == 0) {
        return "00:00:00";
    }

    std::chrono::milliseconds current_time_ms{interval_num * msPerInterval()};
    auto current_time_rounded_sec =
        std::chrono::floor<std::chrono::seconds>(current_time_ms);

    if (interval_num % 2 == 0) {
        auto with_milli_precision =
            subtitler::FormatDuration(current_time_rounded_sec);
        // remove ".000" from the end
        if (int remove_from_here = with_milli_precision.length() - 4;
            remove_from_here > 0) {
            with_milli_precision.erase(remove_from_here);
        }
        return QString::fromStdString(with_milli_precision);
    }

    return "";
}

void Ruler::onZoomIn(int level) {
    zoom_level_ = level;
    // Give user visual confirmation that they have zoomed in.
    interval_width_ += 5;
    updateChildren();
}

void Ruler::onZoomOut(int level) {
    zoom_level_ = level;
    // Give user visual confirmation that they have zoomed out.
    interval_width_ -= 5;
    updateChildren();
}

void Ruler::onAddSubtitleInterval() {
    SubtitleIntervalArgs args;
    args.start_time = indicator_time_;
    args.start_x = millisecondsToPosition(args.start_time);
    args.start_y = HEADER_HEIGHT;

    args.end_time = indicator_time_ + 5s;
    args.end_x = millisecondsToPosition(args.end_time);
    args.end_y = HEADER_HEIGHT;
    subtitle_intervals_->AddInterval(
        std::make_unique<SubtitleInterval>(args, this));
}

void Ruler::drawScaleRuler(QPainter* painter, QRectF ruler_rect) {
    qreal ruler_end_mark = ruler_rect.right();

    for (qreal current = origin_; current <= ruler_end_mark;
         current += 2 * interval_width_) {
        qreal x1 = current;
        qreal y1 = ruler_rect.top() + HEADER_HEIGHT - 5;
        qreal x2 = current;
        qreal y2 = ruler_rect.bottom();

        // draw 2 tickers within one circle.
        QPen ticker_pen(QColor(61, 61, 61), 1);
        painter->setPen(ticker_pen);
        painter->drawLine(QLineF(x1, y1, x2, y2));
        if (x1 + interval_width_ <= ruler_end_mark) {
            painter->drawLine(
                QLineF(x1 + interval_width_, y1, x2 + interval_width_, y2));
        }

        // draw 2 time text within one circle.
        QPen text_pen(QColor(121, 121, 121), 1);
        painter->setPen(text_pen);
        painter->drawText(x1 - TIME_LABEL_OFFSET, y1 - HEADER_HEIGHT / 4,
                          getTickerString(x1));
        if (x1 + interval_width_ - TIME_LABEL_OFFSET <= ruler_end_mark) {
            painter->drawText(x1 + interval_width_ - TIME_LABEL_OFFSET,
                              y1 - HEADER_HEIGHT / 4,
                              getTickerString(x1 + interval_width_));
        }
    }
}
