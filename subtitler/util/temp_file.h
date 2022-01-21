#ifndef SUBTITLER_UTIL_TEMP_FILE_H
#define SUBTITLER_UTIL_TEMP_FILE_H

#include <filesystem>
#include <string>

namespace subtitler {

/**
 * Simple wrapper for writing data to a temp file and ensuring it is deleted.
 * There are various security implications to creating and managing temp files.
 * For simplicity this does not defend against those attacks. Do not use this
 * code with sensitive data. A more secure implementation would be to use
 * platform specific APIs to restrict file access.
 */
class TempFile {
  public:
    // Constructor takes in the string data to write, and handles creating a
    // temp file with this data. Throws std::runtime_error if something goes.
    explicit TempFile(const std::string& data,
                      const std::filesystem::path& parent_path,
                      const std::string& extension);

    // When this object is destroyed, the temp file will be deleted.
    ~TempFile();

    TempFile(const TempFile& other) = delete;
    TempFile& operator=(const TempFile& other) = delete;

    // Returns the temporary file name.
    std::string FileName() const { return temp_file_name_; }

    // The filename but with windows backslash and colon escaped.
    // Ex. C:\Windows\fonts becomes C\:/Windows/fonts.
    // This is needed to pass this filename as an argument to FFPlay etc.
    std::string EscapedFileName() const { return escaped_temp_file_name_; }

  private:
    std::string temp_file_name_;
    std::string escaped_temp_file_name_;
};

}  // namespace subtitler

#endif
