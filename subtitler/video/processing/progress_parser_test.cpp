#include "subtitler/video/processing/progress_parser.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

using subtitler::video::processing::Progress;
using subtitler::video::processing::ProgressParser;

using namespace std::chrono_literals;

const char* PROGRESS_TEST_OUTPUT_CONTINUE = R"(
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

const char* PROGRESS_TEST_OUTPUT_END = R"(
frame=123457
fps=191.05
stream_0_0_q=32.0
bitrate=1417.3kbits/s
total_size=83623981
out_time_us=472339751
out_time_ms=472339751
out_time=00:07:52.339751
dup_frames=124
drop_frames=457
speed=7.29x
progress=end
)";

const char* PROGRESS_TEST_OUTPUT_BREAK_ON_LINE_A = R"(
frame=123456
fps=190.05
stream_0_0_q=31.0
bitrate=1416.3kbits/s
total_size=83623980
out_time_us=472339750
)";

const char* PROGRESS_TEST_OUTPUT_BREAK_ON_LINE_B = R"(
out_time_ms=472339750
out_time=00:07:52.339750
dup_frames=123
drop_frames=456
speed=6.29x
progress=continue
)";

TEST(ProgressParserTest, ParseTwoCompleteUpdates) {
    ProgressParser parser;
    auto result_opt = parser.Receive(PROGRESS_TEST_OUTPUT_CONTINUE);
    ASSERT_TRUE(result_opt);
    auto result = *result_opt;

    ASSERT_EQ(result.frame, 123456);
    ASSERT_EQ(result.fps, 190.05);
    ASSERT_EQ(result.bitrate, "1416.3kbits/s");
    ASSERT_EQ(result.total_size, 83623980);
    ASSERT_EQ(result.out_time_us, 472339750us);
    ASSERT_EQ(result.dup_frames, 123);
    ASSERT_EQ(result.drop_frames, 456);
    ASSERT_EQ(result.speed, "6.29x");
    ASSERT_EQ(result.progress, "continue");

    result_opt = parser.Receive(PROGRESS_TEST_OUTPUT_END);
    ASSERT_TRUE(result_opt);
    result = *result_opt;

    ASSERT_EQ(result.frame, 123457);
    ASSERT_EQ(result.fps, 191.05);
    ASSERT_EQ(result.bitrate, "1417.3kbits/s");
    ASSERT_EQ(result.total_size, 83623981);
    ASSERT_EQ(result.out_time_us, 472339751us);
    ASSERT_EQ(result.dup_frames, 124);
    ASSERT_EQ(result.drop_frames, 457);
    ASSERT_EQ(result.speed, "7.29x");
    ASSERT_EQ(result.progress, "end");
}

TEST(ProgressParserTest, ParseIncompleteUpdate_BreakOnLine) {
    ProgressParser parser;
    auto res_opt1 = parser.Receive(PROGRESS_TEST_OUTPUT_BREAK_ON_LINE_A);
    ASSERT_FALSE(res_opt1);

    auto res_opt2 = parser.Receive(PROGRESS_TEST_OUTPUT_BREAK_ON_LINE_B);
    ASSERT_TRUE(res_opt2);

    const auto& result = *res_opt2;

    ASSERT_EQ(result.frame, 123456);
    ASSERT_EQ(result.fps, 190.05);
    ASSERT_EQ(result.bitrate, "1416.3kbits/s");
    ASSERT_EQ(result.total_size, 83623980);
    ASSERT_EQ(result.out_time_us, 472339750us);
    ASSERT_EQ(result.dup_frames, 123);
    ASSERT_EQ(result.drop_frames, 456);
    ASSERT_EQ(result.speed, "6.29x");
    ASSERT_EQ(result.progress, "continue");
}

TEST(ProgressParserTest, ParseIncompleteUpdate_BreakInMiddle) {
    // Take BREAK_ON_LINE_A and BREAK_ON_LINE_B, but further break them up
    // in the middle. So total 4 parts.
    std::string break_a_wrapper{PROGRESS_TEST_OUTPUT_BREAK_ON_LINE_A};
    std::string break_b_wrapper{PROGRESS_TEST_OUTPUT_BREAK_ON_LINE_B};

    std::string section1 =
        break_a_wrapper.substr(0, break_a_wrapper.length() / 2);
    std::string section2 = break_a_wrapper.substr(break_a_wrapper.length() / 2);
    std::string section3 =
        break_b_wrapper.substr(0, break_b_wrapper.length() / 2);
    std::string section4 = break_b_wrapper.substr(break_b_wrapper.length() / 2);

    // We want to test cases where break does not happen on a new line.
    // So just sanity check that's what happened here.
    ASSERT_NE(section1.back(), '\n');
    ASSERT_NE(section3.back(), '\n');

    ProgressParser parser;
    auto res_opt = parser.Receive(section1.c_str());
    ASSERT_FALSE(res_opt);
    res_opt = parser.Receive(section2.c_str());
    ASSERT_FALSE(res_opt);
    res_opt = parser.Receive(section3.c_str());
    ASSERT_FALSE(res_opt);
    res_opt = parser.Receive(section4.c_str());
    ASSERT_TRUE(res_opt);

    const auto& result = *res_opt;

    ASSERT_EQ(result.frame, 123456);
    ASSERT_EQ(result.fps, 190.05);
    ASSERT_EQ(result.bitrate, "1416.3kbits/s");
    ASSERT_EQ(result.total_size, 83623980);
    ASSERT_EQ(result.out_time_us, 472339750us);
    ASSERT_EQ(result.dup_frames, 123);
    ASSERT_EQ(result.drop_frames, 456);
    ASSERT_EQ(result.speed, "6.29x");
    ASSERT_EQ(result.progress, "continue");
}
