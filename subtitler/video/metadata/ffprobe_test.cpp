#include "subtitler/video/metadata/ffprobe.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <nlohmann/json.hpp>

#include "subtitler/subprocess/mock_subprocess_executor.h"

using subtitler::subprocess::MockSubprocessExecutor;
using subtitler::video::metadata::FFProbe;
using ::testing::_;
using ::testing::InSequence;
using ::testing::IsEmpty;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;
using namespace std::chrono_literals;

TEST(FFProbeTest, ContainerHasAudioAndVideoStream) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    auto json_with_audio_stream = R"({
        "streams": [{
            "index": 0,
            "codec_name": "aac",
            "codec_type": "audio",
            "sample_rate": "48000",
            "channels": 2,
            "start_time": "0.000000",
            "duration": "1200.234000"
        }, {
            "index": 1,
            "codec_name": "h264",
            "codec_type": "video",
            "codec_tag_string": "avc1",
            "width": 1280,
            "height": 720,
            "has_b_frames": 2,
            "start_time": "0.000000",
            "duration": "1239.195111"
        }]
    })";
    {
        InSequence sequence;
        EXPECT_CALL(
            *mock_executor,
            SetCommand(
                "ffprobe \"video.mp4\" -hide_banner -show_format"
                " -loglevel error -show_streams -print_format json=compact=1"))
            .Times(1);
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(5000)))
            .Times(1)
            .WillOnce(Return(
                MockSubprocessExecutor::Output{json_with_audio_stream, ""}));
    }

    FFProbe ffprobe("ffprobe", std::move(mock_executor));
    auto metadata = ffprobe.GetVideoMetadata("video.mp4");

    ASSERT_TRUE(metadata->audio.has_value());
    const auto &audio = metadata->audio;
    ASSERT_EQ(audio->index, 0);
    ASSERT_EQ(audio->codec_name, "aac");
    ASSERT_EQ(audio->sample_rate, 48000);
    ASSERT_EQ(audio->channels, 2);
    ASSERT_EQ(audio->start_time, 0ms);
    ASSERT_EQ(audio->duration, 1200s + 234ms);

    ASSERT_TRUE(metadata->video.has_value());
    const auto &video = metadata->video;
    ASSERT_EQ(video->index, 1);
    ASSERT_EQ(video->codec_name, "h264");
    ASSERT_EQ(video->codec_tag_string, "avc1");
    ASSERT_EQ(video->width, 1280);
    ASSERT_EQ(video->height, 720);
    ASSERT_EQ(video->has_b_frames, 2);
    ASSERT_EQ(video->start_time, 0ms);
    ASSERT_EQ(video->duration, 1239s + 195ms);
}

TEST(FFProbeTest, FFProbeStderrIsThrownBackAtCaller) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(5000)))
            .Times(1)
            .WillOnce(
                Return(MockSubprocessExecutor::Output{"stdout", "stderr"}));
    }

    try {
        FFProbe ffprobe("ffprobe", std::move(mock_executor));
        auto metadata = ffprobe.GetVideoMetadata("video.mp4");
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error &e) {
        ASSERT_STREQ(e.what(), "Error running ffprobe: stderr");
    }
}

TEST(FFProbeTest, EmptyJsonThrowsError) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    {
        InSequence sequence;
        EXPECT_CALL(*mock_executor, Start()).Times(1);
        EXPECT_CALL(*mock_executor, WaitUntilFinished(std::optional<int>(5000)))
            .Times(1)
            .WillOnce(Return(MockSubprocessExecutor::Output{"{}", ""}));
    }

    try {
        FFProbe ffprobe("ffprobe", std::move(mock_executor));
        auto metadata = ffprobe.GetVideoMetadata("video.mp4");
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error &e) {
        ASSERT_STREQ(e.what(), "FFProbe could not read streams info");
    }
}

TEST(FFProbeTest, EmptyVideoPathThrowsError) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    try {
        FFProbe ffprobe("ffprobe", std::move(mock_executor));
        auto metadata = ffprobe.GetVideoMetadata("");
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument &e) {
        ASSERT_STREQ(e.what(), "Video path is empty");
    }
}

TEST(FFProbeTest, InvalidConstructorArgumentsThrowsInvalidArgument) {
    auto mock_executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
    try {
        FFProbe ffprobe("", std::move(mock_executor));
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument &e) {
        ASSERT_STREQ(e.what(), "FFProbe path provided cannot be empty!");
    }

    try {
        FFProbe ffprobe("ffprobe", nullptr);
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument &e) {
        ASSERT_STREQ(e.what(), "Executor provided to ffprobe cannot be null!");
    }
}
