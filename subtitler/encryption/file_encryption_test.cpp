#include "subtitler/encryption/file_encryption.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "subtitler/util/temp_file.h"

using namespace subtitler;
namespace fs = std::filesystem;

TEST(FileEncryption, RoundTrip) {
    std::string plain_text = "This is some plaintext. Blah blah!";
    std::string password = "password123";
    std::string temp_dir = std::getenv("TEST_TMPDIR");

    TempFile file{"", temp_dir, ".encrypt"};
    encryption::EncryptDataToFile(file.FileName(), plain_text, password);
    auto decrypted = encryption::DecryptFromFile(file.FileName(), password);

    EXPECT_EQ(decrypted, plain_text);
}

TEST(FileEncryption, WrongPassword_Throws) {
    std::string plain_text = "This is some plaintext. Blah blah!";
    std::string password = "password123";
    std::string temp_dir = std::getenv("TEST_TMPDIR");

    TempFile file{"", temp_dir, ".encrypt"};
    encryption::EncryptDataToFile(file.FileName(), plain_text, password);

    try {
        auto decrypted =
            encryption::DecryptFromFile(file.FileName(), "somethingwrong");
        FAIL() << "Expected exception to be thrown";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "corrupted chunk");
    }
}

TEST(FileEncryption, EmptyPlaintext_Throws) {
    std::string temp_dir = std::getenv("TEST_TMPDIR");
    TempFile file{"", temp_dir, ".encrypt"};
    try {
        encryption::EncryptDataToFile(file.FileName(), "", "something");
        FAIL() << "Expected exception to be thrown";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "data cannot be empty!");
    }
}

TEST(FileEncryption, EmptyPassword_Throws) {
    std::string temp_dir = std::getenv("TEST_TMPDIR");
    TempFile file{"", temp_dir, ".encrypt"};
    try {
        encryption::EncryptDataToFile(file.FileName(), "something", "");
        FAIL() << "Expected exception to be thrown";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "password cannot be empty!");
    }
    try {
        encryption::DecryptFromFile(file.FileName(), "");
        FAIL() << "Expected exception to be thrown";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "password cannot be empty!");
    }
}

TEST(FileEncryption, EmptyFilePath_Throws) {
    try {
        encryption::EncryptDataToFile("", "something", "abc");
        FAIL() << "Expected exception to be thrown";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "output_path cannot be empty!");
    }
    try {
        encryption::DecryptFromFile("", "abc");
        FAIL() << "Expected exception to be thrown";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "input_path cannot be empty!");
    }
}
