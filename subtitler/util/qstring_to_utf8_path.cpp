#include "subtitler/util/qstring_to_utf8_path.h"

#include <filesystem>
#include <stdexcept>

namespace subtitler {

std::filesystem::path QStringToUtf8Path(const QString& qstr) {
  QByteArray qstr_u8 = qstr.toUtf8();
  std::u8string u8_path{qstr_u8.begin(), qstr_u8.end()};
  return std::filesystem::path{u8_path};
}

}  // namespace subtitler
