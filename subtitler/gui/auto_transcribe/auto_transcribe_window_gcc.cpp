#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QThreadPool>
#include <stdexcept>

#include "subtitler/gui/auto_transcribe/auto_transcribe_window.h"

namespace subtitler {
namespace gui {
namespace auto_transcribe {

namespace {

const char* TRANSCRIBE_SERVICE_EXPLANATION =
    "Sorry! Auto Transcribe is only available on Windows right now! Hopefully "
    "we can bring it to Linux Soon! See "
    "https://github.com/Novacer/SubTite-add-subtitles-to-videos for more "
    "details.";

}  // namespace

AutoTranscribeWindow::AutoTranscribeWindow(Inputs inputs, QWidget* parent)
    : QDialog{parent}, inputs_{inputs}, can_close_{true} {
  setWindowTitle(tr("Auto Transcribe"));
  setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  QGridLayout* layout = new QGridLayout{this};
  layout->setVerticalSpacing(10);

  transcribe_service_explanation_ =
      new QLabel{tr(TRANSCRIBE_SERVICE_EXPLANATION), this};
  transcribe_service_explanation_->setWordWrap(true);
  layout->addWidget(transcribe_service_explanation_, 0, 0, 3, 3);
}

AutoTranscribeWindow::~AutoTranscribeWindow() = default;

void AutoTranscribeWindow::onTranscribe() {
  // TODO
  onTranscribeComplete("");
}

void AutoTranscribeWindow::onProgressUpdate(const QString progress) {
  // TODO
}

void AutoTranscribeWindow::onTranscribeComplete(QString error) {
  // TODO handle the error.
  can_close_ = true;
  accept();
}

void AutoTranscribeWindow::accept() {
  if (can_close_) {
    QDialog::accept();
  }
}

void AutoTranscribeWindow::reject() {
  if (can_close_) {
    QDialog::reject();
  }
}

}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler
