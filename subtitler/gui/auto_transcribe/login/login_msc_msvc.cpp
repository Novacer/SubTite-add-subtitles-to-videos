#include "subtitler/gui/auto_transcribe/login/login_msc.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <Qlabel>
#include <QGridLayout>

namespace subtitler {
namespace gui {
namespace auto_transcribe {
namespace login {

LoginMicrosoftCognitiveServicesWindow::LoginMicrosoftCognitiveServicesWindow(
    QWidget* parent)
    : QDialog{parent} {
    setWindowTitle(tr("Login to Microsoft Cognitive Service"));
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QString encrypted_data_path =
        QCoreApplication::applicationDirPath() + "/mcs.encrypt";
    
    QGridLayout *layout = new QGridLayout{this};
    layout->setVerticalSpacing(10);

    // The file exists as a directory... something went wrong!
    if (QDir{encrypted_data_path}.exists()) {
        explanation_ = new QLabel{
            tr("An issue was detected with your previous Microsoft Cognitive "
               "Services setup. You will need to setup again."),
            this};
        setup_again_button_ = new QPushButton{tr("Setup Again"), this};

        layout->addWidget(explanation_, 0, 0, 3, 3);
        layout->addWidget(setup_again_button_, 3, 2, Qt::AlignRight);
    }
    // The file exists and is not a directory
    else if (QFileInfo::exists(encrypted_data_path)) {
        explanation_ = new QLabel{tr("Please enter your password."), this};
        // Login flow
        password_label_ = new QLabel{tr("Password: "), this};
        password_edit_ = new QLineEdit{this};

        setup_again_button_ = new QPushButton{tr("Setup Again"), this};
        submit_button_ = new QPushButton{tr("Submit"), this};

        layout->addWidget(explanation_, 0, 0, 3, 3);
        layout->addWidget(password_label_, 3, 0, 1, 1);
        layout->addWidget(password_edit_, 3, 1, 1, 2);
        layout->addWidget(setup_again_button_, 4, 1, Qt::AlignLeft);
        layout->addWidget(submit_button_, 4, 2, Qt::AlignRight);
    } else {
        // Register flow
        explanation_ = new QLabel{
            tr("Please enter your Microsoft Speech Services API key and API "
               "region. If you need help, please refer to the README: "
               "https://github.com/Novacer/SubTite-add-subtitles-to-videos. "
               "This will only be stored on your computer, and will be "
               "encrypted with the password you provide here."),
            this};
        api_key_label_ = new QLabel{tr("API Key: "), this};
        api_key_edit_ = new QLineEdit{this};

        api_region_label_ = new QLabel{tr("API Region: "), this};
        api_region_edit_ = new QLineEdit{this};

        password_label_ = new QLabel{tr("Password: "), this};
        password_edit_ = new QLineEdit{this};

        submit_button_ = new QPushButton{tr("Submit"), this};

        layout->addWidget(explanation_, 0, 0, 3, 3);
        layout->addWidget(api_key_label_, 3, 0, 1, 1);
        layout->addWidget(api_key_edit_, 3, 1, 1, 2);
        layout->addWidget(api_region_label_, 4, 0, 1, 1);
        layout->addWidget(api_region_edit_, 4, 1, 1, 2);
        layout->addWidget(password_label_, 5, 0, 1, 1);
        layout->addWidget(password_edit_, 5, 1, 1, 2);
        layout->addWidget(submit_button_, 6, 2, Qt::AlignRight);
    }
    explanation_->setWordWrap(true);
}

QString LoginMicrosoftCognitiveServicesWindow::GetLoginData() const {
    return login_data_;
}

}  // namespace login
}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler
