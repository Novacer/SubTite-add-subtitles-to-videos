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

TEST(FFProbeTest, ContainerHasAudioAndVideoStream) {
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
