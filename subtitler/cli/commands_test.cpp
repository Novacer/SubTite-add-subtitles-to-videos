#include "subtitler/cli/commands.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <streambuf>

#include "subtitler/cli/io/input.h"
#include "subtitler/subprocess/mock_subprocess_executor.h"
#include "subtitler/util/font_config.h"
#include "subtitler/video/player/ffplay.h"

using subtitler::cli::Commands;
using subtitler::cli::io::NarrowInputGetter;
using subtitler::subprocess::MockSubprocessExecutor;
using subtitler::video::player::FFPlay;
using ::testing::_;
using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::NiceMock;
using ::testing::Not;
using ::testing::Return;
using ::testing::Throw;

std::unique_ptr<NarrowInputGetter> CreateInputGetter(std::istream &stream) {
    return std::make_unique<NarrowInputGetter>(stream);
}

class CommandsTest : public ::testing::Test {
  protected:
    void SetUp() override {
        auto executor = std::make_unique<NiceMock<MockSubprocessExecutor>>();
        mock_executor = executor.get();
        video_path = "path/to/test.mp4";
        ffplay_path = "path/to/ffplay";
        srt_path = "test.srt";
        // Clear the file beforehand.
        std::ofstream file{srt_path, std::ofstream::out | std::ofstream::trunc};

        ffplay = std::make_unique<FFPlay>(ffplay_path, std::move(executor));
        paths = Commands::Paths{video_path, srt_path};
    }

    MockSubprocessExecutor *mock_executor;
    std::string video_path;
    std::string ffplay_path;
    std::string srt_path;
    std::unique_ptr<FFPlay> ffplay;
    Commands::Paths paths;
};

TEST_F(CommandsTest, HelpReturnsNonEmptyMessage) {
    std::istringstream input{"\n \n  help"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(),
                HasSubstr("help -- Prints the supported commands"));
}

TEST_F(CommandsTest, CommandNotRecognizedPrintsToStdout) {
    std::istringstream input{"something123"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(),
                HasSubstr("Command something123 not recognized!"));
}

TEST_F(CommandsTest, PlayCorrectlySetsStartAndDuration) {
    std::istringstream input{"play start 1:30.5 duration 45"};
    std::ostringstream output;
    std::ostringstream expected_command;
    expected_command
        << ffplay_path << " " << video_path << " "
        << "-sn -ss 00:01:30.500 -t 00:00:45.000 -left 0 -top 0 -vf "
        << "\"drawtext=text='%{pts\\:hms}':fontsize=(h/"
           "30):fontcolor=white:box=1:boxcolor=black:fontfile='"
        << subtitler::get_font_path() << "'\" -loglevel error";
    EXPECT_CALL(*mock_executor, SetCommand(expected_command.str())).Times(1);

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();
}

TEST_F(CommandsTest, PlayCorrectlySetsStartAndDuration_Swapped) {
    std::istringstream input{"play duration 45.12 start 1:23:45"};
    std::ostringstream output;
    std::ostringstream expected_command;
    expected_command
        << ffplay_path << " " << video_path << " "
        << "-sn -ss 01:23:45.000 -t 00:00:45.120 -left 0 -top 0 -vf "
        << "\"drawtext=text='%{pts\\:hms}':fontsize=(h/"
           "30):fontcolor=white:box=1:boxcolor=black:fontfile='"
        << subtitler::get_font_path() << "'\" -loglevel error";
    EXPECT_CALL(*mock_executor, SetCommand(expected_command.str())).Times(1);

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_MissingStartTime) {
    std::istringstream input{"play start"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Missing start time!"));
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_UnableToParseStartTime) {
    std::istringstream input{"play start duration 123"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Unable to parse start time!"));
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_MissingDurationTime) {
    std::istringstream input{"play duration"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Missing duration time!"));
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_UnableToParseDurationTime) {
    std::istringstream input{"play duration 123abc"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Unable to parse duration time!"));
}

TEST_F(CommandsTest, PlayWarnsUserOfIncorrectUsage_UnrecognizedToken) {
    std::istringstream input{"play start 123 duration 456 wacky"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Unrecognized token: wacky"));
}

TEST_F(CommandsTest, PlayPrintsErrorWhenClosingPlayerReturnsError) {
    std::istringstream input{"play\nplay"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, WaitUntilFinished(_))
        .Times(1)
        .WillOnce(Return(MockSubprocessExecutor::Output{"stdout", "stderr"}));

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Error closing player: stderr"));
}

TEST_F(CommandsTest, PlayPrintsErrorWhenOpeningPlayerReturnsError) {
    std::istringstream input{"play"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, Start())
        .Times(1)
        .WillOnce(Throw(std::runtime_error("some error message")));

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(),
                HasSubstr("Error opening player: some error message"));
}

TEST_F(CommandsTest, PlayNextPrintsErrorWhenClosingPlayerReturnsError) {
    std::istringstream input{"play \n play next"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, WaitUntilFinished(_))
        .Times(1)
        .WillOnce(Return(MockSubprocessExecutor::Output{"stdout", "stderr"}));

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Error closing player: stderr"));
}

TEST_F(CommandsTest, PlayNextCorrectlyUpdatesNewStartAndDuration) {
    std::istringstream input{"play start 1 duration 10 \n play next"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, WaitUntilFinished(_))
        .Times(1)
        .WillOnce(Return(MockSubprocessExecutor::Output{"stdout", ""}));

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(),
                HasSubstr("Playing start=00:00:11.000 duration=00:00:05.000"));
}

TEST_F(CommandsTest, QuitClosesOpenPlayersAndBreaksTheLoop) {
    std::istringstream input{"play start 1 \n quit \n play"};
    std::ostringstream output;
    EXPECT_CALL(*mock_executor, Start()).Times(1);
    EXPECT_CALL(*mock_executor, WaitUntilFinished(_))
        .Times(1)
        .WillOnce(
            Return(MockSubprocessExecutor::Output{"stdout", "I am closed."}));

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Error closing player: I am closed."));
}

TEST_F(CommandsTest, AddedSubtitleIsThenPrintable) {
    std::istringstream input{
        "add p middle-right \nsome subtitle\n\n printsubs"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("1\n"
                                        "00:00:00,000 --> 00:00:05,000\n"
                                        "{\\an6} some subtitle\n"
                                        "\n"));
}

TEST_F(CommandsTest, EmptySubtitleIsNotAdded) {
    std::istringstream input{"add p bottom-left \n\n printsubs"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), Not(HasSubstr(" --> ")));
}

TEST_F(CommandsTest, AddSubCanBeCancelledInFlight) {
    std::istringstream input{
        "add p bottom-left \nHello world!\n/cancel \n printsubs"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), Not(HasSubstr("Hello world!")));
    ASSERT_THAT(output.str(), HasSubstr("Enter next command:"));
}

TEST_F(CommandsTest, AddedSubtitleCanReplayVideoDuringInput) {
    std::istringstream input{
        "add p middle-right \nline 1\n/play \nline 2\n\n printsubs"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    // Use output log to check if play was called.
    // Other way is to test SetCommand param, but this will contain the temp
    // file path which is hard to predict. Either way this test might be a bit
    // brittle, so just do the easier one for now.
    ASSERT_THAT(
        output.str(),
        HasSubstr("Playing start=00:00:00.000 duration=00:00:05.000\n"));

    ASSERT_THAT(output.str(), HasSubstr("1\n"
                                        "00:00:00,000 --> 00:00:05,000\n"
                                        "{\\an6} line 1\n"
                                        "line 2\n"
                                        "\n"));
}

TEST_F(CommandsTest, AddedSubtitleIsThenSaveable) {
    std::istringstream input{"add p middle-right \nsome subtitle\n\n save"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    std::ifstream ifs{srt_path};
    std::string file((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());

    ASSERT_EQ(file,
              "1\n"
              "00:00:00,000 --> 00:00:05,000\n"
              "{\\an6} some subtitle\n"
              "\n");
}

TEST_F(CommandsTest, AddedSubtitleCanBeSavedWhileQuitting) {
    std::istringstream input{"add p top-center \nsome subtitle\n\n quit \n Y"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    std::ifstream ifs{srt_path};
    std::string file((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());

    ASSERT_THAT(output.str(), HasSubstr("Save before closing? Input: [Y/n]"));
    ASSERT_EQ(file,
              "1\n"
              "00:00:00,000 --> 00:00:05,000\n"
              "{\\an8} some subtitle\n"
              "\n");
}

TEST_F(CommandsTest, AddSubInvalidCommandsPrintsErrorMessages) {
    std::istringstream input{
        "add position \n add position invalid \n add random stuff"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(),
                HasSubstr("Missing position id. Valid positions are:"));
    ASSERT_THAT(
        output.str(),
        HasSubstr("Position invalid not recognized. Valid positions are: "));
    ASSERT_THAT(output.str(), HasSubstr("Unrecognized token: random"));
}

TEST_F(CommandsTest, DeleteSubInRangeCanBeDoneWithoutForce) {
    std::istringstream input{"add p top-center \nsome subtitle\n\n delete 1"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Deleted: 1\n"
                                        "00:00:00,000 --> 00:00:05,000\n"
                                        "{\\an8} some subtitle\n"
                                        "\n"));
}

TEST_F(CommandsTest, DeleteSubOutOfRangeCannotBeDoneWithoutForce) {
    std::istringstream input{
        "add p top-center \nsome subtitle\n\n play next \n play next \n delete "
        "1"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(),
                HasSubstr("The subtitle you want to delete is not within the "
                          "current player position."));
    ASSERT_THAT(output.str(), Not(HasSubstr("Deleted: 1\n"
                                            "00:00:00,000 --> 00:00:05,000\n"
                                            "{\\an8} some subtitle\n"
                                            "\n")));
}

TEST_F(CommandsTest, DeleteSubOutOfRangeWithForce) {
    std::istringstream input{
        "add p top-center \nsome subtitle\n\n delete 1 --force"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("Deleted: 1\n"
                                        "00:00:00,000 --> 00:00:05,000\n"
                                        "{\\an8} some subtitle\n"
                                        "\n"));
}

TEST_F(CommandsTest, DeleteSubInvalidCommandsPrintErrorMessages) {
    std::istringstream input{"delete 1 2 \n delete --force \n delete invalid"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(),
                HasSubstr("Can only provide one sequence number to delete!"));
    ASSERT_THAT(output.str(),
                HasSubstr("Missing sequence num. Check help for usage."));
    ASSERT_THAT(output.str(), HasSubstr("Unrecognized token: invalid"));
}

TEST_F(CommandsTest, EditSubPositionAfterAdding) {
    std::istringstream input{
        "add p top-center \nsome subtitle\n\n edit 1 position bottom-left \n "
        "printsubs"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(), HasSubstr("1\n"
                                        "00:00:00,000 --> 00:00:05,000\n"
                                        "{\\an1} some subtitle\n"
                                        "\n"));
}

TEST_F(CommandsTest, EditSubPositionInvalidCommandsPrintErrorMessages) {
    std::istringstream input{
        "add p tc \nsubtitle\n\n edit \n edit invalid \n"
        "edit 1 invalid \n edit 1 p \n edit 1 p invalid \n edit 9999 p "
        "middle-center"};
    std::ostringstream output;

    Commands commands{paths, std::move(ffplay), CreateInputGetter(input),
                      output};
    commands.MainLoop();

    ASSERT_THAT(output.str(),
                HasSubstr("Missing sequence num. Check help for usage."));
    ASSERT_THAT(output.str(),
                HasSubstr("Unable to parse invalid as sequence number!"));
    ASSERT_THAT(output.str(), HasSubstr("Unrecognized token: invalid"));
    ASSERT_THAT(output.str(),
                HasSubstr("Missing position id. Valid positions are:"));
    ASSERT_THAT(
        output.str(),
        HasSubstr("Unable to edit position of 1. Valid positions are:"));
    ASSERT_THAT(
        output.str(),
        HasSubstr("Unable to edit position of 9999. Valid positions are:"));
}
