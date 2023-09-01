#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "subtitler/encryption/file_encryption.h"
#include "subtitler/gui/auto_transcribe/login/login_msc.h"

namespace subtitler {
namespace gui {
namespace auto_transcribe {
namespace login {

namespace {

QString ENCRYPTED_FILE_NAME = "mcs.encrypt";

}  // namespace

LoginMicrosoftCognitiveServicesWindow::LoginMicrosoftCognitiveServicesWindow(
    QWidget* parent)
    : QDialog{parent}, num_times_failed_{0} {
    setWindowTitle(tr("Login to Microsoft Cognitive Service"));
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QString encrypted_data_path =
        QCoreApplication::applicationDirPath() + "/" + ENCRYPTED_FILE_NAME;

    QGridLayout* layout = new QGridLayout{this};
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

        connect(setup_again_button_, &QPushButton::clicked, this,
                &LoginMicrosoftCognitiveServicesWindow::onSetupAgain);
    }
    // The file exists and is not a directory
    else if (QFileInfo::exists(encrypted_data_path)) {
        explanation_ = new QLabel{tr("Please enter your password."), this};
        // Login flow
        password_label_ = new QLabel{tr("Password: "), this};
        password_edit_ = new QLineEdit{this};
        password_edit_->setEchoMode(QLineEdit::Password);

        setup_again_button_ = new QPushButton{tr("Setup Again"), this};
        submit_button_ = new QPushButton{tr("Submit"), this};

        layout->addWidget(explanation_, 0, 0, 3, 3);
        layout->addWidget(password_label_, 3, 0, 1, 1);
        layout->addWidget(password_edit_, 3, 1, 1, 2);
        layout->addWidget(setup_again_button_, 4, 1, Qt::AlignLeft);
        layout->addWidget(submit_button_, 4, 2, Qt::AlignRight);

        connect(setup_again_button_, &QPushButton::clicked, this,
                &LoginMicrosoftCognitiveServicesWindow::onSetupAgain);
        connect(submit_button_, &QPushButton::clicked, this,
                &LoginMicrosoftCognitiveServicesWindow::onLogin);
        // So that pressing enter submits.
        setup_again_button_->setDefault(false);
        submit_button_->setDefault(true);
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
        password_edit_->setEchoMode(QLineEdit::Password);

        submit_button_ = new QPushButton{tr("Submit"), this};

        layout->addWidget(explanation_, 0, 0, 3, 3);
        layout->addWidget(api_key_label_, 3, 0, 1, 1);
        layout->addWidget(api_key_edit_, 3, 1, 1, 2);
        layout->addWidget(api_region_label_, 4, 0, 1, 1);
        layout->addWidget(api_region_edit_, 4, 1, 1, 2);
        layout->addWidget(password_label_, 5, 0, 1, 1);
        layout->addWidget(password_edit_, 5, 1, 1, 2);
        layout->addWidget(submit_button_, 6, 2, Qt::AlignRight);

        connect(submit_button_, &QPushButton::clicked, this,
                &LoginMicrosoftCognitiveServicesWindow::onRegister);
    }
    explanation_->setWordWrap(true);
}

QString LoginMicrosoftCognitiveServicesWindow::GetLoginData() const {
    return login_data_;
}

void LoginMicrosoftCognitiveServicesWindow::onRegister() {
    if (!api_key_edit_ || !api_region_edit_ || !password_edit_) {
        return;
    }
    if (api_key_edit_->text().isEmpty() || api_region_edit_->text().isEmpty() ||
        password_edit_->text().isEmpty()) {
        return;
    }
    nlohmann::json registration_data;
    registration_data["api-key"] = api_key_edit_->text().toStdString();
    registration_data["api-region"] = api_region_edit_->text().toStdString();
    QString output_file =
        QCoreApplication::applicationDirPath() + "/" + ENCRYPTED_FILE_NAME;

    std::string password = password_edit_->text().toStdString();

    try {
        encryption::EncryptDataToFile(output_file.toStdString(),
                                      registration_data.dump(), password);
        // Test decryption round trip
        std::string payload =
            encryption::DecryptFromFile(output_file.toStdString(), password);
        nlohmann::json decrypted_data = nlohmann::json::parse(payload);
        if (decrypted_data != registration_data) {
            throw std::runtime_error{
                "Attempt to encrypt failed round-trip check"};
        }
        login_data_ = QString::fromStdString(payload);

        accept();
    } catch (const std::exception& e) {
        ++num_times_failed_;
        if (num_times_failed_ < 3) {
            explanation_->setText(
                tr("Try again after deleting mcs.encrypt in the same folder as "
                   "subtite.exe, or file a bug?"));
        } else {
            explanation_->setText(tr("Error: ") + e.what());
        }
    }
}

void LoginMicrosoftCognitiveServicesWindow::onLogin() {
    if (!password_edit_ || password_edit_->text().isEmpty()) {
        return;
    }

    std::string password = password_edit_->text().toStdString();
    QString input_file =
        QCoreApplication::applicationDirPath() + "/" + ENCRYPTED_FILE_NAME;

    try {
        std::string payload =
            encryption::DecryptFromFile(input_file.toStdString(), password);
        nlohmann::json decrypted_data = nlohmann::json::parse(payload);
        login_data_ = QString::fromStdString(payload);

        accept();
    } catch (const std::exception& e) {
        ++num_times_failed_;
        if (num_times_failed_ < 3) {
            explanation_->setText(tr("Error: Wrong password?"));
        } else {
            explanation_->setText(tr("Try setting up again?"));
        }
    }
}

void LoginMicrosoftCognitiveServicesWindow::onSetupAgain() {
    QString input_file =
        QCoreApplication::applicationDirPath() + "/" + ENCRYPTED_FILE_NAME;
    QFile::remove(input_file);
    reject();
}

}  // namespace login
}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler
