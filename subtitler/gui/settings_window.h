#ifndef SUBTITLER_GUI_SETTINGS_WINDOW_H
#define SUBTITLER_GUI_SETTINGS_WINDOW_H

#include <QDialog>
#include <QString>

namespace subtitler {
namespace gui {

struct Settings {
    QString video_file;
    QString subtitle_file;
};

Settings GetSettings(const Settings &current_settings);

class SettingsWindow : public QDialog {
    Q_OBJECT
  public:
    SettingsWindow(Settings &settings, QWidget *parent = Q_NULLPTR);
    ~SettingsWindow() = default;

  private:
    Settings& settings_;
};

}  // namespace gui
}  // namespace subtitler

#endif
