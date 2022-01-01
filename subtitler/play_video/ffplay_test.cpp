#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "subtitler/play_video/ffplay.h"
#include "subtitler/subprocess/mock_subprocess_executor.h"

using subtitler::play_video::FFPlay;
using subtitler::subprocess::MockSubprocessExecutor;
using ::testing::IsEmpty;


TEST(FFPlayTest, OpenPlayerWithDefaultArgs) {
    auto mock_executor = std::make_unique<MockSubprocessExecutor>();
    std::string expected_command = "ffplay video.mp4 -sn -loglevel error";
    EXPECT_CALL(*mock_executor, SetCommand(expected_command));

    FFPlay ffplay("ffplay", std::move(mock_executor));
    ffplay.OpenPlayer("video.mp4");
}
