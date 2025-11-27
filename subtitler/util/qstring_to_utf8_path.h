#ifndef SUBTITLER_UTIL_QSTRING_TO_UTF8_PATH_H
#define SUBTITLER_UTIL_QSTRING_TO_UTF8_PATH_H

#include <QString>
#include <filesystem>
#include <string>

namespace subtitler {

// Helper to handle interaction between QString and the filesystem.
// In Windows, filepaths are handled as UTF-16 wide strings, while Unix
// uses UTF-8 narrow strings. This function forces the QString to be converted
// to a UTF-8, and then lets std::filesystem::path handle the rest.
std::filesystem::path QStringToUtf8Path(const QString& qstr);

}  // namespace subtitler

#endif  // SUBTITLER_UTIL_QSTRING_TO_UTF8_PATH_H
