#include "subtitler/gui/subtitle_editor.h"

#include <QPlainTextEdit>

#include "subtitler/gui/timeline/subtitle_interval.h"

SubtitleEditor::SubtitleEditor(QWidget* parent)
    : QDockWidget{parent}, currently_editing_{Q_NULLPTR} {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    text_edit_ = new QPlainTextEdit(this);
    text_edit_->setMinimumSize(200, 100);
    text_edit_->setPlaceholderText(tr("Subtitle text here"));

    connect(text_edit_, &QPlainTextEdit::textChanged, this,
            &SubtitleEditor::onSubtitleTextChanged);

    setWidget(text_edit_);
}

void SubtitleEditor::onOpenSubtitle(SubtitleInterval* subtitle) {
    currently_editing_ = subtitle;
    text_edit_->setPlainText(subtitle->GetSubtitleText());
    setVisible(true);
}

void SubtitleEditor::onSubtitleTextChanged() {
    if (!currently_editing_) {
        return;
    }
    currently_editing_->SetSubtitleText(text_edit_->toPlainText());
}
