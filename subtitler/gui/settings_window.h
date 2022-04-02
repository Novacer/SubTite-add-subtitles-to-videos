#ifndef SUBTITLER_GUI_SETTINGS_WINDOW_H
#define SUBTITLER_GUI_SETTINGS_WINDOW_H

#include <QDialog>
#include <QString>

QT_FORWARD_DECLARE_CLASS(QLabel)

namespace subtitler {
namespace gui {

struct Settings {
    QString video_file;
    QString subtitle_file;
};

/**
 * Open the settings dialog and block until the user closes the dialog.
 *
 * @param current_settings the current settings to populate the dialog.
 * @return Settings the new settings the user has selected.
 */
Settings GetSettings(const Settings &current_settings);

/**
 * A dialog for changing the settings.
 */
class SettingsWindow : public QDialog {
    Q_OBJECT
  public:
    SettingsWindow(Settings &settings, QWidget *parent = Q_NULLPTR);
    ~SettingsWindow() = default;

  private:
    Settings &settings_;
    QLabel *error_msg_;
};

}  // namespace gui
}  // namespace subtitler

#endif
