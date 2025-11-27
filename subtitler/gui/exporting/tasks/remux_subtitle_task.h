#ifndef SUBTITLER_GUI_EXPORTING_TASKS_REMUX_SUBTITLE_TASK
#define SUBTITLER_GUI_EXPORTING_TASKS_REMUX_SUBTITLE_TASK

#include <QRunnable>
#include <QString>

#include "subtitler/gui/exporting/export_dialog.h"

namespace subtitler {
namespace gui {
namespace exporting {
namespace tasks {

class RemuxSubtitleTask : public QRunnable {
 public:
  RemuxSubtitleTask(QString video, QString subtitle, QString output,
                    ExportWindow* parent);

  void run() override;

 private:
  QString video_;
  QString subtitle_;
  QString output_;
  ExportWindow* parent_;
};

}  // namespace tasks
}  // namespace exporting
}  // namespace gui
}  // namespace subtitler

#endif
