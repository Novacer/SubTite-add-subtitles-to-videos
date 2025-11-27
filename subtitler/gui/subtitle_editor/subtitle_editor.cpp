#include "subtitler/gui/subtitle_editor/subtitle_editor.h"

#include <QDebug>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <sstream>

#include "subtitler/gui/subtitle_editor/position_buttons.h"
#include "subtitler/gui/timeline/subtitle_interval.h"
#include "subtitler/util/duration_format.h"

namespace subtitler {
namespace gui {
namespace subtitle_editor {

namespace {

QString FormatSubtitleStartEndString(
    const timeline::SubtitleInterval* subtitle) {
  std::ostringstream builder;
  builder << subtitler::ToSubRipDuration(subtitle->GetBeginTime()) << " --> "
          << subtitler::ToSubRipDuration(subtitle->GetEndTime());
  return QString::fromStdString(builder.str());
}

}  // namespace

SubtitleEditor::SubtitleEditor(QWidget* parent)
    : QDockWidget{parent},
      currently_editing_{Q_NULLPTR},
      container_{Q_NULLPTR},
      prev_visibility_{false} {
  setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

  QWidget* main_placeholder = new QWidget(this);

  begin_end_time_ = new QLabel(main_placeholder);

  text_edit_ = new QPlainTextEdit(main_placeholder);
  text_edit_->setPlaceholderText(tr("Subtitle text here"));
  text_edit_->installEventFilter(this);

  position_buttons_ = new PositionButtons{main_placeholder};

  QWidget* delete_save_placeholder = new QWidget(main_placeholder);
  QPushButton* save_button = new QPushButton(delete_save_placeholder);
  save_button->setText(tr("Save"));
  QPushButton* delete_button = new QPushButton(delete_save_placeholder);
  delete_button->setText(tr("Delete"));

  QHBoxLayout* delete_save_layout = new QHBoxLayout(delete_save_placeholder);
  delete_save_layout->addWidget(delete_button, 50, Qt::AlignLeft);
  delete_save_layout->addWidget(save_button, 50, Qt::AlignRight);

  QVBoxLayout* layout = new QVBoxLayout(main_placeholder);
  layout->addWidget(begin_end_time_);
  layout->addWidget(text_edit_);
  layout->addWidget(position_buttons_);
  layout->addWidget(delete_save_placeholder);

  setWidget(main_placeholder);

  connect(text_edit_, &QPlainTextEdit::textChanged, this,
          &SubtitleEditor::onSubtitleTextChanged);
  connect(save_button, &QPushButton::clicked, this,
          [this]() { this->onSave(); });
  connect(delete_button, &QPushButton::clicked, this,
          &SubtitleEditor::onDelete);
  connect(this, &SubtitleEditor::visibilityChanged, this,
          &SubtitleEditor::onVisibilityChanged);
  connect(position_buttons_, &PositionButtons::positionIdSelected, this,
          &SubtitleEditor::onPositionSelected);
}

std::size_t SubtitleEditor::GetNumSubtitles() const {
  if (!container_) {
    return 0;
  }
  return container_->intervals().size();
}

void SubtitleEditor::onOpenSubtitle(
    timeline::SubtitleIntervalContainer* container,
    timeline::SubtitleInterval* subtitle) {
  currently_editing_ = subtitle;
  container_ = container;
  if (!subtitle) {
    text_edit_->setPlainText("");
    begin_end_time_->setText("");
    return;
  }
  text_edit_->setPlainText(subtitle->GetSubtitleText());
  begin_end_time_->setText(FormatSubtitleStartEndString(subtitle));
  position_buttons_->onPositionChanged(subtitle->GetSubtitlePosition());
  setVisible(true);
}

void SubtitleEditor::onSubtitleTextChanged() {
  if (!currently_editing_) {
    return;
  }
  currently_editing_->SetSubtitleText(text_edit_->toPlainText());
}

void SubtitleEditor::onSubtitleChangeStartEndTime(
    timeline::SubtitleInterval* subtitle) {
  if (subtitle != currently_editing_) {
    // irrelevant event, skip.
    return;
  }
  begin_end_time_->setText(FormatSubtitleStartEndString(subtitle));
}

// Null container means to keep using the container member.
// A valid container ptr means to use the new container for successive calls.
void SubtitleEditor::onSave(timeline::SubtitleIntervalContainer* container) {
  if (!container_ && !container) {
    return;
  }
  if (container) {
    container_ = container;
  }
  container_->SaveSubripFile();
  emit saved(container_->intervals().size());
}

void SubtitleEditor::onDelete() {
  if (!container_ || !currently_editing_) {
    return;
  }
  timeline::SubtitleInterval* backup = currently_editing_;
  currently_editing_ = Q_NULLPTR;
  container_->RemoveInterval(backup);
  // Closing editor should also save the file.
  setVisible(false);
}

// Save when closing editor.
void SubtitleEditor::onVisibilityChanged(bool visible) {
  if (prev_visibility_ != visible && !visible) {
    onSave();
  }
  prev_visibility_ = visible;
}

bool SubtitleEditor::eventFilter(QObject* watched, QEvent* event) {
  if (watched == text_edit_ && event->type() == QEvent::FocusOut) {
    onSave();
  }
  return false;
}

void SubtitleEditor::onPositionSelected(const std::string& position_id) {
  if (!currently_editing_) {
    return;
  }
  currently_editing_->SetSubtitlePosition(position_id);
  onSave();
}

void SubtitleEditor::onSubtitleFileReload(const QString& new_subtitle_file) {
  currently_editing_ = Q_NULLPTR;
  container_ = Q_NULLPTR;
  text_edit_->setPlainText("");
  begin_end_time_->setText("");
  setVisible(false);
}

}  // namespace subtitle_editor
}  // namespace gui
}  // namespace subtitler
