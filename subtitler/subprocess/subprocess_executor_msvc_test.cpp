#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "subtitler/subprocess/subprocess_executor.h"

using subtitler::subprocess::SubprocessExecutor;
using ::testing::IsEmpty;

TEST(SubprocessExecutorTest, SanityCheck) {
    SubprocessExecutor executor;
    executor.SetCommand("echo hello world");
    executor.CaptureStdout(true);

    executor.Start();
    auto captured_stdout = executor.WaitUntilFinished();

    ASSERT_EQ(captured_stdout, "hello world\n");
}

TEST(SubprocessExecutorTest, DoNotCaptureStdout) {
    SubprocessExecutor executor;
    executor.SetCommand("echo hello world");
    executor.CaptureStdout(false);

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
        ASSERT_STREQ(e.what(), "You must wait for subprocess to finish before starting again.");
    }
}
