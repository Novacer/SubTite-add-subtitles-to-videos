#include "subtitler/video/processing/ffmpeg.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

#include "subtitler/subprocess/mock_subprocess_executor.h"

using subtitler::subprocess::MockSubprocessExecutor;
using subtitler::video::processing::FFMpeg;
using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

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
    {
        InSequence sequence;
        EXPECT_CALL(
            *mock_executor,
            SetCommand(
                "ffmpeg -i 'video.mp4' -vf subtitles='C\\:/foo/subtitle.srt' "
                "'output.mp4' -loglevel error -progress pipe:1 -stats_period "
                "5"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(1000)))
            .Times(1)
            .WillOnce(Return(MockSubprocessExecutor::Output{"", ""}));
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    ffmpeg.BurnSubtitlesAsync("video.mp4", "C:\\foo\\subtitle.srt",
                              "output.mp4");
    ffmpeg.WaitForAsyncTask(1000);
}

TEST(FFMpegTest, BurnSubtitlesAsync_TwiceThrowsError) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        EXPECT_CALL(
            *mock_executor,
            SetCommand(
                "ffmpeg -i 'video.mp4' -vf subtitles='subtitle.srt' "
                "'output.mp4' -loglevel error -progress pipe:1 -stats_period "
                "5"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    ffmpeg.BurnSubtitlesAsync("video.mp4", "subtitle.srt", "output.mp4");
    try {
        ffmpeg.BurnSubtitlesAsync("somethingelse.mp4", "foo.srt", "bar.mp4");
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ(
            e.what(),
            "You must call FFMpeg::WaitForAsyncTask() before executing another "
            "task!");
    }
}

TEST(FFMpegTest, WaitForAsyncTask_ThrowsStdErr) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        EXPECT_CALL(
            *mock_executor,
            SetCommand(
                "ffmpeg -i 'video.mp4' -vf subtitles='subtitle.srt' "
                "'output.mp4' -loglevel error -progress pipe:1 -stats_period "
                "5"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>{}))
            .Times(1)
            .WillOnce(Return(MockSubprocessExecutor::Output{"", "some error"}));
    }

    FFMpeg ffmpeg("ffmpeg", std::move(mock_executor));
    ffmpeg.BurnSubtitlesAsync("video.mp4", "subtitle.srt", "output.mp4");
    try {
        ffmpeg.WaitForAsyncTask();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        ASSERT_STREQ(e.what(), "Error running ffmpeg: some error");
    }
}
