#include "subtitler/util/temp_file.h"

#include <glog/logging.h>

#include <cstdio>
#include <filesystem>
#include <random>
#include <sstream>
#include <stdexcept>

#include "subtitler/util/unicode.h"

#ifdef _MSC_VER
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace subtitler {

namespace {

std::string GetRandomString(int length) {
    std::string random_str;
    random_str.reserve(length);
    static std::string chrs =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type>
        pick(0, chrs.length() - 1);

    while (length--) {
        random_str += chrs.at(pick(rg));
    }

    return random_str;
}

}  // namespace

#ifdef _MSC_VER
TempFile::TempFile(const std::string &data, const fs::path &parent_path,
                   const std::string &extension) {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    std::string random_file_name;
    int count = 0;
    while (hFile == INVALID_HANDLE_VALUE) {
        if (count > 10) {
            throw std::runtime_error("Could not write random path!");
        }
        auto random_str = GetRandomString(/* length= */ 10);
        auto random_full_path = parent_path / (random_str + extension);
        random_file_name = random_full_path.string();
        std::wstring wide_file_name = ConvertToWString(random_file_name);
        if (wide_file_name.empty()) {
            throw std::runtime_error{"Could not convert temp filename to wstr"};
        }

        hFile = CreateFileW(wide_file_name.c_str(),  // name of the write
                            GENERIC_WRITE,           // open for writing
                            0,                       // do not share
                            NULL,                    // default security
                            CREATE_NEW,              // create new file only
                            FILE_ATTRIBUTE_NORMAL,   // normal file
                            NULL);                   // no attr. template
        count++;
    }
    LOG(INFO) << "Created a temp file " << random_file_name;
    DWORD bytes_written = 0;
    auto bErrorFlag =
        WriteFile(hFile,           // open file handle
                  data.c_str(),    // start of data to write
                  data.length(),   // number of bytes to write
                  &bytes_written,  // number of bytes that were written
                  NULL);           // no overlapped structure
    if (!bErrorFlag || bytes_written != data.length()) {
        throw std::runtime_error("Failed to write to temp file");
    }
    CloseHandle(hFile);

    temp_file_name_ = random_file_name;
}

#else
TempFile::TempFile(const std::string &data, const fs::path &parent_path,
                   const std::string &extension) {
    FILE *fp = nullptr;
    std::string random_file_name;

    int count = 0;
    while (!fp) {
        if (count > 10) {
            throw std::runtime_error("Could not write random path!");
        }

        auto random_str = GetRandomString(/* length= */ 10);
        auto random_full_path = parent_path / (random_str + extension);
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
}
#endif

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
