#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <Qlabel>

#include "subtitler/gui/auto_transcribe/login/login_msc.h"

namespace subtitler {
namespace gui {
namespace auto_transcribe {
namespace login {

namespace {

const char* EXPLANATION =
    "Sorry! Auto Transcribe is only available on Windows right now! Hopefully "
    "we can bring it to Linux Soon! See "
    "https://github.com/Novacer/SubTite-add-subtitles-to-videos for more "
    "details.";

}  // namespace

LoginMicrosoftCognitiveServicesWindow::LoginMicrosoftCognitiveServicesWindow(
    QWidget* parent)
    : QDialog{parent}, num_times_failed_{0} {
    setWindowTitle(tr("Login to Microsoft Cognitive Service"));
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QGridLayout* layout = new QGridLayout{this};
    layout->setVerticalSpacing(10);

    explanation_ = new QLabel{tr(EXPLANATION), this};
    explanation_->setWordWrap(true);
    layout->addWidget(explanation_, 0, 0, 3, 3);
}

QString LoginMicrosoftCognitiveServicesWindow::GetLoginData() const {
    return login_data_;
}

void LoginMicrosoftCognitiveServicesWindow::onRegister() {}

void LoginMicrosoftCognitiveServicesWindow::onLogin() {}

void LoginMicrosoftCognitiveServicesWindow::onSetupAgain() {}

}  // namespace login
}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler
