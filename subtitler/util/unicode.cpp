#include "subtitler/util/unicode.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <filesystem>
#include <stdexcept>

namespace subtitler {

std::filesystem::path GetFileSystemUtf8Path(const std::string& utf8_path) {
  std::u8string u8_path{utf8_path.begin(), utf8_path.end()};
  return std::filesystem::path{u8_path};
}

#ifdef _MSC_VER
std::string ConvertFromWString(const std::wstring& wstr) {
  int num_chars = WideCharToMultiByte(
      /* CodePage= */ CP_UTF8,
      /* dwFlags= */ WC_ERR_INVALID_CHARS,
      /* lpWideCharStr= */ wstr.c_str(),
      /* cchWideChar= */ wstr.length(),
      /* lpMultiByteStr= */ NULL,
      /* cbMultiByte= */ 0,
      /* lpDefaultChar= */ NULL,
      /* lpUsedDefaultChar= */ NULL);

  if (num_chars <= 0) {
    return std::string();
  }

  std::string result;
  result.resize(num_chars);
  auto res = WideCharToMultiByte(
      /* CodePage= */ CP_UTF8,
      /* dwFlags= */ WC_ERR_INVALID_CHARS,
      /* lpWideCharStr= */ wstr.c_str(),
      /* cchWideChar= */ wstr.length(),
      /* lpMultiByteStr= */ &result[0],
      /* cbMultiByte= */ num_chars,
      /* lpDefaultChar= */ NULL,
      /* lpUsedDefaultChar= */ NULL);
  if (!res) {
    throw std::runtime_error("Could not convert wstr to str.");
  }

  return result;
}

// Converts narrow string (utf-8) to wide string (utf-16).
std::wstring ConvertToWString(const std::string& str) {
  int num_chars = MultiByteToWideChar(
      /* CodePage= */ CP_UTF8,
      /* dwFlags= */ MB_ERR_INVALID_CHARS,
      /* lpMultiByteStr= */ str.c_str(),
      /* cbMultiByte= */ str.length(),
      /* lpWideCharStr= */ NULL,
      /* cchWideChar= */ 0);

  if (num_chars <= 0) {
    return std::wstring();
  }

  std::wstring result;
  result.resize(num_chars);
  auto res = MultiByteToWideChar(
      /* CodePage= */ CP_UTF8,
      /* dwFlags= */ MB_ERR_INVALID_CHARS,
      /* lpMultiByteStr= */ str.c_str(),
      /* cbMultiByte= */ str.length(),
      /* lpWideCharStr= */ &result[0],
      /* cchWideChar= */ num_chars);
  if (!res) {
    throw std::runtime_error("Could not convert str to wstr.");
  }

  return result;
}

#endif  // _MSC_VER

}  // namespace subtitler
