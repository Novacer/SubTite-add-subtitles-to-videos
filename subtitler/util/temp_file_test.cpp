#include "subtitler/util/temp_file.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

using namespace subtitler;

TEST(TempFileTest, CreateTempFileGetsDeleted) {
    std::string data = "hello world!\nthis is a test :)\n";
    std::string file_name;
    {
        TempFile file(data);
        file_name = file.FileName();
        std::ifstream ifs{file_name};
        std::string contents((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());

        ASSERT_EQ(contents, data);
    }
    // File goes out of scope, test if it still exists.
    ASSERT_FALSE(std::filesystem::exists(file_name));
}
