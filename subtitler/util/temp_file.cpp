#include "subtitler/util/temp_file.h"

#include <cstdio>
#include <stdexcept>
#include <filesystem>
#include <sstream>

namespace subtitler {

namespace {

// Replaces backwards slashes with forward slashes
// Replaces C: with C\:. Needed to make windows file paths work with ffmpeg.
std::string FixPath(const std::string &path) {
    std::ostringstream output;
    for (const auto &c: path) {
        if (c == '\\') {
            output << '/';
        } else if (c == ':') {
            output << "\\:";
        } else {
            output << c;
        }
    }
    return output.str();
}

} // namespace

TempFile::TempFile(const std::string &data) {
    FILE* fp = nullptr;
    char random_file_name[L_tmpnam_s];
    while (!fp) {
        // Generate a random & currently unique file name.
        // However due to TOCTOU this is not guaranteed to be unique at time of file creation.
        // Furthermore, symlink attack is also possible.
        // We attempt to mitigate this by opening in x mode, which fails if file already exists.
        // Detailed discussion: https://stackoverflow.com/questions/14230886
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
    temp_file_name_ = FixPath(random_file_name);
}

TempFile::~TempFile() {
    // Must use non-throwing version.
    std::error_code ec;
    std::filesystem::remove(temp_file_name_, ec);
}

} // namespace subtitler
