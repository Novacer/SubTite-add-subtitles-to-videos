#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "subtitler/subprocess/subprocess_executor.h"

using subtitler::subprocess::SubprocessExecutor;
using ::testing::IsEmpty;

TEST(SubprocessExecutorTest, SanityCheck) {
    SubprocessExecutor executor(
        /* command= */ "echo hello world",
        /* capture_output= */ true);

    executor.Start();
    auto captured_stdout = executor.WaitUntilFinished();

    ASSERT_EQ(captured_stdout, "hello world\n");

    executor.SetCommand("echo try again!");
    executor.Start();
    captured_stdout = executor.WaitUntilFinished(5000);

    ASSERT_EQ(captured_stdout, "try again!\n");
}

TEST(SubprocessExecutor, UTF8StringTest) {
    SubprocessExecutor executor;
    executor.SetCommand(u8"echo 你好世界");
    executor.CaptureOutput(true);

    executor.Start();
    auto captured_stdout = executor.WaitUntilFinished();

    ASSERT_EQ(captured_stdout, u8"你好世界\n");
}

TEST(SubprocessExecutorTest, DoNotCaptureStdout) {
    SubprocessExecutor executor;
    executor.SetCommand("echo hello world");
    executor.CaptureOutput(false);

    executor.Start();
    auto captured_stdout = executor.WaitUntilFinished();

    ASSERT_THAT(captured_stdout, IsEmpty());
}

TEST(SubprocessExecutor, StartTwiceWithoutWaitingThrowsError) {
    SubprocessExecutor executor;
    executor.SetCommand("echo hello world");

    executor.Start();
    try {
        executor.Start();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ(e.what(), "You must call WaitUntilFinished() before starting again.");
    }
}

TEST(SubprocessExecutor, InvalidCommandThrowsError_EmptyString) {
    SubprocessExecutor executor;
    executor.SetCommand("");

    try {
        executor.Start();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ(e.what(), "Unable to create process to run: ");
    }
}

TEST(SubprocessExecutor, InvalidCommandThrowsError_CommandDoesNotExist) {
    SubprocessExecutor executor;
    executor.SetCommand("DoesNotExist");

    try {
        executor.Start();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ(e.what(), "Unable to create process to run: DoesNotExist");
    }
}
