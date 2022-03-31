#include "subtitler/video/processing/ffmpeg.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <functional>

#include "subtitler/subprocess/mock_subprocess_executor.h"
#include "subtitler/video/processing/progress_parser.h"

using subtitler::subprocess::MockSubprocessExecutor;
using subtitler::video::processing::FFMpeg;
using subtitler::video::processing::Progress;
using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;

using namespace std::chrono_literals;

const char* PROGRESS_TEST_OUTPUT = R"(
frame=123456
fps=190.05
stream_0_0_q=31.0
bitrate=1416.3kbits/s
total_size=83623980
out_time_us=472339750
out_time_ms=472339750
out_time=00:07:52.339750
dup_frames=123
drop_frames=456
speed=6.29x
progress=continue
)";

TEST(FFMpegTest, GetVersionInfo_ReturnsStdOut) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    std::string mock_expected_version = "ffmpeg version 4.4.0 ...";
    {
        InSequence sequence;
        EXPECT_CALL(*mock_executor,
                    SetCommand("ffmpeg -version -loglevel error"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(5000)))
            .Times(1)
            .WillOnce(Return(
                MockSubprocessExecutor::Output{mock_expected_version, ""}));
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    ASSERT_EQ(ffmpeg.GetVersionInfo(), mock_expected_version);
}

TEST(FFMpegTest, GetVersionInfo_ThrowsStdErr) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        EXPECT_CALL(*mock_executor,
                    SetCommand("ffmpeg -version -loglevel error"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(5000)))
            .Times(1)
            .WillOnce(Return(MockSubprocessExecutor::Output{"", "some error"}));
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    try {
        ffmpeg.GetVersionInfo();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ(e.what(), "Error running ffmpeg: some error");
    }
}

TEST(FFMpegTest, BurnSubtitlesAsync_Success) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    std::function<void(const char*)> intercepted_callback;
    {
        InSequence sequence;
        EXPECT_CALL(
            *mock_executor,
            SetCommand(
                "ffmpeg -y -i \"video.mp4\" -vf "
                "\"subtitles='C\\:/foo/subtitle.srt'\" "
                "\"output.mp4\" -loglevel error -progress pipe:1 -stats_period "
                "5"))
            .Times(1);
        EXPECT_CALL(*mock_executor, SetCallback)
            .WillOnce(SaveArg<0>(&intercepted_callback));
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(1000)))
            .Times(1)
            .WillOnce(Return(MockSubprocessExecutor::Output{"", ""}));
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    bool callback_run = false;
    ffmpeg.BurnSubtitlesAsync("video.mp4", "C:\\foo\\subtitle.srt",
                              "output.mp4",
                              [&callback_run](const Progress& progress) {
                                  ASSERT_EQ(progress.frame, 123456);
                                  ASSERT_EQ(progress.fps, 190.05);
                                  ASSERT_EQ(progress.bitrate, "1416.3kbits/s");
                                  ASSERT_EQ(progress.total_size, 83623980);
                                  ASSERT_EQ(progress.out_time_us, 472339750us);
                                  ASSERT_EQ(progress.dup_frames, 123);
                                  ASSERT_EQ(progress.drop_frames, 456);
                                  ASSERT_EQ(progress.speed, "6.29x");
                                  ASSERT_EQ(progress.progress, "continue");
                                  callback_run = true;
                              });

    intercepted_callback(PROGRESS_TEST_OUTPUT);

    ffmpeg.WaitForAsyncTask(1000);

    ASSERT_TRUE(callback_run);
}

TEST(FFMpegTest, BurnSubtitlesAsync_TwiceThrowsError) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        EXPECT_CALL(
            *mock_executor,
            SetCommand(
                "ffmpeg -y -i \"video.mp4\" -vf \"subtitles='subtitle.srt'\" "
                "\"output.mp4\" -loglevel error -progress pipe:1 -stats_period "
                "5"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    ffmpeg.BurnSubtitlesAsync("video.mp4", "subtitle.srt", "output.mp4", {});
    try {
        ffmpeg.BurnSubtitlesAsync("somethingelse.mp4", "foo.srt", "bar.mp4",
                                  {});
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ(
            e.what(),
            "You must call FFMpeg::WaitForAsyncTask() before executing another "
            "task!");
    }
}

TEST(FFMpegTest, RemuxSubtitlesAsync_Success) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    std::function<void(const char*)> intercepted_callback;
    {
        InSequence sequence;
        EXPECT_CALL(
            *mock_executor,
            SetCommand(
                "ffmpeg -y -i \"video.mp4\" -i \"C:\\foo\\subtitle.srt\" "
                "-map 0 -map 1:s -c copy "
                "\"output.mkv\" -loglevel error -progress pipe:1 -stats_period "
                "5"))
            .Times(1);
        EXPECT_CALL(*mock_executor, SetCallback)
            .WillOnce(SaveArg<0>(&intercepted_callback));
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(1000)))
            .Times(1)
            .WillOnce(Return(MockSubprocessExecutor::Output{"", ""}));
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    bool callback_run = false;
    ffmpeg.RemuxSubtitlesAsync("video.mp4", "C:\\foo\\subtitle.srt",
                               "output.mkv",
                               [&callback_run](const Progress& progress) {
                                   ASSERT_EQ(progress.frame, 123456);
                                   ASSERT_EQ(progress.fps, 190.05);
                                   ASSERT_EQ(progress.bitrate, "1416.3kbits/s");
                                   ASSERT_EQ(progress.total_size, 83623980);
                                   ASSERT_EQ(progress.out_time_us, 472339750us);
                                   ASSERT_EQ(progress.dup_frames, 123);
                                   ASSERT_EQ(progress.drop_frames, 456);
                                   ASSERT_EQ(progress.speed, "6.29x");
                                   ASSERT_EQ(progress.progress, "continue");
                                   callback_run = true;
                               });

    intercepted_callback(PROGRESS_TEST_OUTPUT);

    ffmpeg.WaitForAsyncTask(1000);

    ASSERT_TRUE(callback_run);
}

TEST(FFMpegTest, WaitForAsyncTask_ThrowsStdErr) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        EXPECT_CALL(
            *mock_executor,
            SetCommand(
                "ffmpeg -y -i \"video.mp4\" -vf \"subtitles='subtitle.srt'\" "
                "\"output.mp4\" -loglevel error -progress pipe:1 -stats_period "
                "5"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>{}))
            .Times(1)
            .WillOnce(Return(MockSubprocessExecutor::Output{"", "some error"}));
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    ffmpeg.BurnSubtitlesAsync("video.mp4", "subtitle.srt", "output.mp4", {});
    try {
        ffmpeg.WaitForAsyncTask();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ(e.what(), "Error running ffmpeg: some error");
    }
}
