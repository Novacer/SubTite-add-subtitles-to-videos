#include "subtitler/experimental/qt_gui/subtitle_editor.h"

#include <QTextEdit>
#include "subtitler/experimental/qt_gui/timeline/subtitle_interval.h"

SubtitleEditor::SubtitleEditor(QWidget* parent) : QDockWidget{parent} {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    text_edit_ = new QTextEdit(this);
    text_edit_->setMinimumSize(200, 100);
    text_edit_->setPlaceholderText(tr("Subtitle text here"));
    setWidget(text_edit_);
}

void SubtitleEditor::onOpenSubtitle(SubtitleInterval* subtitle) {
    text_edit_->setText(subtitle->GetSubtitleText());
    setVisible(true);
}
