#include "subtitler/util/temp_file.h"

#include <cstdio>
#include <stdexcept>
#include <filesystem>

namespace subtitler {

TempFile::TempFile(const std::string &data) {
    FILE* fp = nullptr;
    char random_file_name[L_tmpnam_s];
    while (!fp) {
        auto err = tmpnam_s(random_file_name, L_tmpnam_s);
        if (err) {
            throw std::runtime_error("Unable to create unique temp file");
        }
        // Write exclusive mode fails if the file already exists.
        // No analog to this in ofstream hence we use the C function instead.
        fp = fopen(random_file_name, "wx");
    }
    // Temp file successfully created. Push the data to it.
    auto err = fputs(data.c_str(), fp);
    if (err) {
        throw std::runtime_error("Failed to write to temp file");
    }
    fclose(fp);
    temp_file_name_ = random_file_name;
}

TempFile::~TempFile() {
    // Must use non-throwing version.
    std::error_code ec;
    std::filesystem::remove(temp_file_name_, ec);
}

} // namespace subtitler
