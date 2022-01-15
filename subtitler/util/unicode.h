#ifndef SUBTITLER_UTIL_UNICODE_H
#define SUBTITLER_UTIL_UNICODE_H

// Functions for MSVC only. No need to convert between wide and narrow strings on Unix.
#ifdef _MSC_VER

#include <string>

namespace subtitler {

// Converts a wide str (utf-16 on windows) to a narrow multi-byte string (utf-8).
// We don't use wstring everywhere to retain some cross-compatibility with linux.
// See http://utf8everywhere.org/
std::string ConvertFromWStr(const std::wstring &wstr);

// Converts narrow string (utf-8) to wide string (utf-16).
std::wstring ConvertToWString(const std::string &str);

} // namespace subtitler

#endif // _MSC_VER
#endif // SUBTITLER_UTIL_UNICODE_H
