#ifndef SUBTITLER_GUI_AUTO_TRANSCRIBE_LOGIN_LOGIN_MSC_H
#define SUBTITLER_GUI_AUTO_TRANSCRIBE_LOGIN_LOGIN_MSC_H

#include <QDialog>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QLineEdit)

namespace subtitler {
namespace gui {
namespace auto_transcribe {
namespace login {

/**
 * Dialog for logging in to using Microsoft Cognitive Services.
 */
class LoginMicrosoftCognitiveServicesWindow : public QDialog {
    Q_OBJECT
  public:
    LoginMicrosoftCognitiveServicesWindow(QWidget* parent = Q_NULLPTR);
    ~LoginMicrosoftCognitiveServicesWindow() = default;

    QString GetLoginData() const;
  
  public slots:
    // User enters API information.
    void onRegister();
    // User enters password to retrieve API information
    void onLogin();
    // User sets up API again.
    void onSetupAgain();

  private:
    QLabel* explanation_;
    QLabel* api_key_label_;
    QLabel* api_region_label_;
    QLabel* password_label_;
    QLineEdit* api_key_edit_;
    QLineEdit* api_region_edit_;
    QLineEdit* password_edit_;
    QPushButton* submit_button_;
    QPushButton* setup_again_button_;

    QString login_data_;
    int num_times_failed_;
};

}  // namespace login
}  // namespace auto_transcribe
}  // namespace gui
}  // namespace subtitler

#endif
