#include "subtitler/video/player/ffplay.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

#include "subtitler/subprocess/mock_subprocess_executor.h"
#include "subtitler/util/font_config.h"

using subtitler::subprocess::MockSubprocessExecutor;
using subtitler::video::player::FFPlay;
using ::testing::InSequence;
using ::testing::IsEmpty;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;

TEST(FFPlayTest, OpenPlayerWithDefaultArgs) {
    auto mock_executor = std::make_unique<StrictMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        // The following must occur in sequence.
        EXPECT_CALL(*mock_executor, CaptureOutput(true)).Times(1);
        EXPECT_CALL(*mock_executor,
                    SetCommand("ffplay \"video.mp4\" -sn -loglevel error"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
    }

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.OpenPlayer("video.mp4");
    ASSERT_TRUE(ffplay.is_playing());
}

TEST(FFPlayTest, OpenPlayerWithScreenDimensionSettings) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    EXPECT_CALL(
        *mock_executor,
        SetCommand(
            "ffplay \"video.mp4\" -x 100 -y 200 -fs -sn -loglevel error"))
        .Times(1);

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.width(100)->height(200)->fullscreen(true)->OpenPlayer("video.mp4");
}

TEST(FFPlayTest, OpenPlayerWithDisableStreamSettings) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    EXPECT_CALL(*mock_executor,
                SetCommand("ffplay \"video.mp4\" -vn -an -loglevel error"))
        .Times(1);

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.disable_video(true)
        ->disable_audio(true)
        ->disable_subtitles(false)
        ->OpenPlayer("video.mp4");
}

TEST(FFPlayTest, OpenPlayerWithSeekAndDurationSettings) {
    using namespace std::chrono_literals;
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    EXPECT_CALL(*mock_executor,
                SetCommand("ffplay \"video.mp4\" -sn -ss 00:00:12.345 -t "
                           "01:23:45.000 -loglevel error"))
        .Times(1);

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.start_pos(12s + 345ms)
        ->duration(1h + 23min + 45s)
        ->OpenPlayer("video.mp4");
}

TEST(FFPlayTest, OpenPlayerWithPositionSettings) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    EXPECT_CALL(
        *mock_executor,
        SetCommand(
            "ffplay \"video.mp4\" -sn -left 100 -top -200 -loglevel error"))
        .Times(1);

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.left_pos(100)->top_pos(-200)->OpenPlayer("video.mp4");
}

TEST(FFPlayTest, OpenPlayerWithTimeStampsEnabled) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    EXPECT_CALL(
        *mock_executor,
        SetCommand("ffplay \"video.mp4\" -sn "
                   "-vf \"drawtext=text='%{pts\\:hms}':"
                   "fontsize=(h/30):fontcolor=white:box=1:boxcolor=black:"
                   "fontfile='" +
                   subtitler::get_font_path() +
                   "'\" "
                   "-loglevel error"))
        .Times(1);

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.enable_timestamp(true)->OpenPlayer("video.mp4");
}

TEST(FFPlayTest, OpenPlayerWithSubtitlesEnabled) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    EXPECT_CALL(*mock_executor, SetCommand("ffplay \"video.mp4\" -sn "
                                           "-vf \"subtitles='subtitle.srt'\" "
                                           "-loglevel error"))
        .Times(1);

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.subtitles_path("subtitle.srt")->OpenPlayer("video.mp4");
}

TEST(FFPlayTest, OpenPlayerWithTimeStampsAndSubtitles) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    EXPECT_CALL(
        *mock_executor,
        SetCommand("ffplay \"video.mp4\" -sn "
                   "-vf \"drawtext=text='%{pts\\:hms}':"
                   "fontsize=(h/30):fontcolor=white:box=1:boxcolor=black:"
                   "fontfile='" +
                   subtitler::get_font_path() +
                   "'"
                   ",subtitles='subtitle.srt'\" "
                   "-loglevel error"))
        .Times(1);

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.subtitles_path("subtitle.srt")
        ->enable_timestamp(true)
        ->OpenPlayer("video.mp4");
}

TEST(FFPlayTest, InvalidConstructorArgumentsThrowsInvalidArgument) {
    try {
        FFPlay ffplay("", std::make_unique<MockSubprocessExecutor>());
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument &e) {
        ASSERT_STREQ(e.what(),
                     "FFPlay path provided to ffplay cannot be empty!");
    }

    try {
        FFPlay ffplay("ffplay", nullptr);
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument &e) {
        ASSERT_STREQ(e.what(), "Executor provided to ffplay cannot be null!");
    }
}

TEST(FFPlayTest, OpenPlayerWithEmptyVideoPathThrowsInvalidArgument) {
    FFPlay ffplay("ffplay", std::make_unique<MockSubprocessExecutor>());
    try {
        ffplay.OpenPlayer("");
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument &e) {
        ASSERT_STREQ(e.what(), "Cannot play empty video path!");
    }
    ASSERT_FALSE(ffplay.is_playing());
}

TEST(FFPlayTest, ClosePlayerReturnsStderr) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(1000)))
            .Times(1)
            .WillOnce(
                Return(MockSubprocessExecutor::Output{"stdout", "stderr"}));
    }

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.OpenPlayer("video.mp4");
    ASSERT_TRUE(ffplay.is_playing());

    auto captured_stderr = ffplay.ClosePlayer(1000);
    ASSERT_FALSE(ffplay.is_playing());
    ASSERT_EQ(captured_stderr, "stderr");
}
