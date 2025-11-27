#ifndef SUBTITLER_UTIL_UNICODE_H
#define SUBTITLER_UTIL_UNICODE_H

#include <filesystem>
#include <string>

namespace subtitler {

// Given a utf-8 encoded path, returns a std::filesystem::path.
std::filesystem::path GetFileSystemUtf8Path(const std::string& utf8_path);

// Functions for MSVC only.
// No need to convert between wide and narrow strings on Unix.
#ifdef _MSC_VER
// Converts a wide str (utf-16 on windows) to a narrow multi-byte string
// (utf-8). We don't use wstring everywhere to retain some cross-compatibility
// with linux. See http://utf8everywhere.org/
std::string ConvertFromWString(const std::wstring& wstr);

// Converts narrow string (utf-8) to wide string (utf-16).
std::wstring ConvertToWString(const std::string& str);

#endif  // _MSC_VER

}  // namespace subtitler

#endif  // SUBTITLER_UTIL_UNICODE_H
