#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "subtitler/subprocess/subprocess_executor.h"

using subtitler::subprocess::SubprocessExecutor;
using ::testing::IsEmpty;

TEST(SubprocessExecutorTest, SanityCheck) {
  SubprocessExecutor executor(
      /* command= */ "echo hello world",
      /* capture_output= */ true);

  bool callback_run = false;
  executor.SetCallback([&callback_run](const char* buffer) {
    std::string line{buffer};
    ASSERT_STREQ(buffer, "hello world\n");
    callback_run = true;
  });

  executor.Start();
  auto captured_ouptut = executor.WaitUntilFinished();

  ASSERT_TRUE(callback_run);

  ASSERT_EQ(captured_ouptut.subproc_stdout, "hello world\n");
  ASSERT_THAT(captured_ouptut.subproc_stderr, IsEmpty());

  executor.SetCommand("cmd /c echo hello world 1>&2");
  executor.Start();
  captured_ouptut = executor.WaitUntilFinished(5000);

  ASSERT_THAT(captured_ouptut.subproc_stdout, IsEmpty());
  // Since we used cmd it will have carriage return.
  ASSERT_THAT(captured_ouptut.subproc_stderr, "hello world \r\n");
}

TEST(SubprocessExecutor, UTF8StringTest) {
  SubprocessExecutor executor;
  executor.SetCommand("echo 你好世界");
  executor.CaptureOutput(true);

  executor.Start();
  auto captured_ouptut = executor.WaitUntilFinished();

  ASSERT_EQ(captured_ouptut.subproc_stdout, "你好世界\n");
  ASSERT_THAT(captured_ouptut.subproc_stderr, IsEmpty());
}

TEST(SubprocessExecutorTest, DoNotCaptureStdout) {
  SubprocessExecutor executor;
  executor.SetCommand("echo hello world");
  executor.CaptureOutput(false);

  executor.Start();
  auto captured_output = executor.WaitUntilFinished();

  ASSERT_THAT(captured_output.subproc_stdout, IsEmpty());
  ASSERT_THAT(captured_output.subproc_stderr, IsEmpty());
}

TEST(SubprocessExecutor, StartTwiceWithoutWaitingThrowsError) {
  SubprocessExecutor executor;
  executor.SetCommand("echo hello world");

  executor.Start();
  try {
    executor.Start();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    ASSERT_STREQ(e.what(),
                 "You must call WaitUntilFinished() before starting again.");
  }
}

TEST(SubprocessExecutor, StopTwiceWithoutStartingThrowsError) {
  SubprocessExecutor executor;
  executor.SetCommand("echo hello world");

  executor.Start();
  try {
    executor.WaitUntilFinished();
    executor.WaitUntilFinished();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    ASSERT_STREQ(e.what(),
                 "You must call Start() before you are able to wait.");
  }
}

TEST(SubprocessExecutor, InvalidCommandThrowsError_EmptyString) {
  SubprocessExecutor executor;
  executor.SetCommand("");

  try {
    executor.Start();
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    ASSERT_STREQ(e.what(), "Cannot start process with empty command!");
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
