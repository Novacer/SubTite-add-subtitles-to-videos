#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>
#include "subtitler/cli/commands.h"
#include "subtitler/subprocess/mock_subprocess_executor.h"
#include "subtitler/play_video/ffplay.h"

using subtitler::play_video::FFPlay;
using subtitler::subprocess::MockSubprocessExecutor;
using subtitler::cli::Commands;
using ::testing::Not;
using ::testing::IsEmpty;
using ::testing::HasSubstr;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;
using ::testing::Throw;

class CommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
        mock_executor = executor.get();
        video_path = "path/to/test.mp4";
        ffplay_path = "path/to/ffplay";
        ffplay = std::make_unique<FFPlay>(ffplay_path, std::move(executor));
        paths = Commands::Paths{video_path};
    }

    MockSubprocessExecutor *mock_executor;
    std::string video_path;
    std::string ffplay_path;
    std::unique_ptr<FFPlay> ffplay;
    Commands::Paths paths;
};

TEST_F(CommandsTest, HelpReturnsNonEmptyMessage) {
    std::istringstream input{"\n \n  help"};
    std::ostringstream output;
    
    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("help -- prints the supported commands"));
}

TEST_F(CommandsTest, CommandNotRecognizedPrintsToStdout) {
    std::istringstream input{"something123"};
    std::ostringstream output;
    
    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Command something123 not recognized!"));
}

TEST_F(CommandsTest, PlayCorrectlySetsStartAndDuration) {
    std::istringstream input{"play start 1:30.5 duration 45"};
    std::ostringstream output;
    std::ostringstream expected_command;
    expected_command << ffplay_path << " "
                     << video_path << " "
                     << "-sn -ss 00:01:30.500 -t 00:00:45.000 -loglevel error";
    EXPECT_CALL(*mock_executor, SetCommand(expected_command.str()))
        .Times(1);

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();
}

TEST_F(CommandsTest, PlayCorrectlySetsStartAndDuration_Swapped) {
    std::istringstream input{"play duration 45.12 start 1:23:45"};
    std::ostringstream output;
    std::ostringstream expected_command;
    expected_command << ffplay_path << " "
                     << video_path << " "
                     << "-sn -ss 01:23:45.000 -t 00:00:45.120 -loglevel error";
    EXPECT_CALL(*mock_executor, SetCommand(expected_command.str()))
        .Times(1);

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_MissingStartTime) {
    std::istringstream input{"play start"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Missing start time!"));
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_UnableToParseStartTime) {
    std::istringstream input{"play start duration 123"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Unable to parse start time!"));
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_MissingDurationTime) {
    std::istringstream input{"play duration"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Missing duration time!"));
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_UnableToParseDurationTime) {
    std::istringstream input{"play duration 123abc"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Unable to parse duration time!"));
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_UnrecognizedToken) {
    std::istringstream input{"play start 123 duration 456 wacky"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Unrecognized token: wacky"));
}

TEST_F(CommandsTest, PlayPrintsErrorWhenClosingPlayerReturnsError) {
    std::istringstream input{"play\nplay"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, WaitUntilFinished(_))
        .Times(1)
        .WillOnce(Return(MockSubprocessExecutor::Output{"stdout", "stderr"}));

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Error closing player: stderr"));
}

TEST_F(CommandsTest, PlayPrintsErrorWhenOpeningPlayerReturnsError) {
    std::istringstream input{"play"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, Start())
        .Times(1)
        .WillOnce(Throw(std::runtime_error("some error message")));

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Error opening player: some error message"));
}

TEST_F(CommandsTest, EndPrintsErrorWhenClosingPlayerReturnsError) {
    std::istringstream input{"play \n end"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, WaitUntilFinished(_))
        .Times(1)
        .WillOnce(Return(MockSubprocessExecutor::Output{"stdout", "stderr"}));

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Error closing player: stderr"));
}

TEST_F(CommandsTest, EndCorrectlyUpdatesNewStartAndDuration) {
    std::istringstream input{"play start 1 duration 10 \n end"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, WaitUntilFinished(_))
        .Times(1)
        .WillOnce(Return(MockSubprocessExecutor::Output{"stdout", ""}));

    Commands commands{paths, std::move(ffplay), input, output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Updated start=00:00:11.000 duration=00:00:05.000"));
}
