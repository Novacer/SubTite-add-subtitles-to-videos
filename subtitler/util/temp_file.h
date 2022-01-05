#ifndef SUBTITLER_UTIL_TEMP_FILE_H
#define SUBTITLER_UTIL_TEMP_FILE_H

#include <string>

namespace subtitler {

// A Simple wrapper for writing data to a temp file and ensuring it is deleted.
// Constructor takes in the string data to write, and handles creating the temp data.
// When this object is destroyed, the temp file will be deleted.
class TempFile {
public:
    explicit TempFile(const std::string &data);
    ~TempFile();
    TempFile(const TempFile &other) = delete;
    TempFile& operator=(const TempFile& other) = delete;

    std::string FileName() const { return temp_file_name_; }

private:
    std::string temp_file_name_;
};

} // namespace subtitler

#endif
