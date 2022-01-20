#include "subtitler/util/temp_file.h"

#include <glog/logging.h>

#include <cstdio>
#include <filesystem>
#include <random>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace subtitler {

namespace {

// Replaces backwards slashes with forward slashes
// Replaces C: with C\:. Needed to make windows file paths work with ffmpeg.
std::string FixPath(const std::string &path) {
    std::ostringstream output;
    for (const auto &c : path) {
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

}  // namespace

TempFile::TempFile(const std::string &data, const fs::path &parent_path,
                   const std::string &extension) {
    FILE *fp = nullptr;
    std::string random_file_name;

    int count = 0;
    while (!fp) {
        if (count > 10) {
            throw std::runtime_error("Could not write random path!");
        }

        static std::string chrs =
            "0123456789"
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        int length = 10;

        thread_local static std::mt19937 rg{std::random_device{}()};
        thread_local static std::uniform_int_distribution<
            std::string::size_type>
            pick(0, chrs.length() - 1);

        random_file_name.clear();
        random_file_name.reserve(length);

        while (length--) {
            random_file_name += chrs.at(pick(rg));
        }

        auto random_full_path = parent_path / (random_file_name + extension);
        random_file_name = random_full_path.string();

        fp = fopen(random_file_name.c_str(), "wx");
        count++;
    }
    LOG(INFO) << "Created a temp file " << random_file_name;
    auto err = fputs(data.c_str(), fp);
    if (err < 0) {
        throw std::runtime_error("Failed to write to temp file");
    }
    fclose(fp);

    temp_file_name_ = random_file_name;
    escaped_temp_file_name_ = FixPath(temp_file_name_);
}

TempFile::~TempFile() {
    // Must use non-throwing version.
    std::error_code ec;
    std::filesystem::remove(temp_file_name_, ec);
    if (ec) {
        LOG(ERROR) << "Failed to delete temp file: " << temp_file_name_
                   << "with error " << ec;
    }
}

}  // namespace subtitler
