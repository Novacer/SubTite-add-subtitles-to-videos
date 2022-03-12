#include "subtitler/gui/subtitle_editor.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <sstream>

#include "subtitler/gui/timeline/subtitle_interval.h"
#include "subtitler/util/duration_format.h"

namespace subtitler {
namespace gui {

namespace {

QString FormatSubtitleStartEndString(const SubtitleInterval* subtitle) {
    std::ostringstream builder;
    builder << subtitler::ToSubRipDuration(subtitle->GetBeginTime()) << " --> "
            << subtitler::ToSubRipDuration(subtitle->GetEndTime());
    return QString::fromStdString(builder.str());
}

}  // namespace

SubtitleEditor::SubtitleEditor(QWidget* parent)
    : QDockWidget{parent}, currently_editing_{Q_NULLPTR} {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* placeholder = new QWidget(this);
    text_edit_ = new QPlainTextEdit(placeholder);
    text_edit_->setMinimumSize(200, 100);
    text_edit_->setPlaceholderText(tr("Subtitle text here"));

    begin_end_time_ = new QLabel(placeholder);
    begin_end_time_->setText("lets gooo");

    QVBoxLayout* layout = new QVBoxLayout(placeholder);
    layout->addWidget(begin_end_time_);
    layout->addWidget(text_edit_);

    setWidget(placeholder);

    connect(text_edit_, &QPlainTextEdit::textChanged, this,
            &SubtitleEditor::onSubtitleTextChanged);
}

void SubtitleEditor::onOpenSubtitle(SubtitleInterval* subtitle) {
    currently_editing_ = subtitle;
    if (!subtitle) {
        text_edit_->setPlainText("");
        begin_end_time_->setText("");
        return;
    }
    text_edit_->setPlainText(subtitle->GetSubtitleText());

    begin_end_time_->setText(FormatSubtitleStartEndString(subtitle));
    setVisible(true);
}

void SubtitleEditor::onSubtitleTextChanged() {
    if (!currently_editing_) {
        return;
    }
    currently_editing_->SetSubtitleText(text_edit_->toPlainText());
}

void SubtitleEditor::onSubtitleChangeStartEndTime(SubtitleInterval* subtitle) {
    if (subtitle != currently_editing_) {
        // irrelevant event, skip.
        return;
    }
    begin_end_time_->setText(FormatSubtitleStartEndString(subtitle));
}

}  // namespace gui
}  // namespace subtitler
