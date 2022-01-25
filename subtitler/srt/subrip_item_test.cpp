#include "subtitler/srt/subrip_item.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>

using namespace std::chrono_literals;
using namespace subtitler;
using namespace subtitler::srt;
using ::testing::HasSubstr;

TEST(SubRipItemTest, MultiLineSubtitle) {
    SubRipItem item;
    item.start(1s + 123ms)
        ->duration(5s)
        ->append_line("This is a subtitle!")
        ->append_line("This is another line!");

    std::ostringstream output;
    item.ToStream(123, output);

    ASSERT_EQ(2, item.num_lines());
    ASSERT_EQ(
        "123\n"
        "00:00:01,123 --> 00:00:06,123\n"
        "This is a subtitle!\n"
        "This is another line!\n",
        output.str());
}

TEST(SubRipItemTest, ClearSubtitle) {
    SubRipItem item;
    item.start(1s + 123ms)
        ->duration(5s)
        ->append_line("This line should not be seen!")
        ->clear_payload()
        ->append_line("This line should be seen!");

    std::ostringstream output;
    item.ToStream(123, output);

    ASSERT_EQ(1, item.num_lines());
    ASSERT_EQ(
        "123\n"
        "00:00:01,123 --> 00:00:06,123\n"
        "This line should be seen!\n",
        output.str());
}

TEST(SubRipItemTest, SetPosition) {
    SubRipItem item;
    item.start(1s + 123ms)->duration(5s)->append_line("Hello World!");

    ASSERT_EQ(1, item.num_lines());

    std::vector<std::string> positions = {
        "bottom-left", "bottom-center", "bottom-right",
        "middle-left", "middle-center", "middle-right",
        "top-left",    "top-center",    "top-right"};
    std::vector<std::string> abbrev_positions = {"bl", "bc", "br", "ml", "mc",
                                                 "mr", "tl", "tc", "tr"};

    auto verify_position_ids =
        [item](const std::vector<std::string> &positions) mutable {
            int expected_pos_id = 1;
            for (const auto &position : positions) {
                std::ostringstream output;
                item.position(position)->ToStream(402, output);
                ASSERT_THAT(
                    output.str(),
                    HasSubstr("{\\an" + std::to_string(expected_pos_id) + "}"));
                ++expected_pos_id;
            }
        };

    verify_position_ids(positions);
    verify_position_ids(abbrev_positions);
}

TEST(SubRipItemTest, SetPositionThrowsOutOfRange) {
    try {
        SubRipItem item;
        item.start(1s + 123ms)
            ->duration(5s)
            ->position("blahblah")
            ->append_line("Hello World!")
            ->append_line("Foo bar baz.");

        FAIL() << "Expected std::out_of_range exception";
    } catch (const std::out_of_range &e) {
    }
}

TEST(SubRipItemTest, Compare) {
    SubRipItem a, b, c;
    a.start(1s)->duration(4s);
    b.start(2000ms)->duration(1s);
    c.start(1s)->duration(2s);

    ASSERT_LT(a, b);
    ASSERT_LT(c, a);
}

TEST(SubRipItemTest, CopySemantics) {
    SubRipItem a, b;
    b.append_line("should not see");
    a.start(1s)
        ->duration(2s)
        ->position("bottom-left")
        ->append_line("hello world");
    b = a;

    std::ostringstream output_b;
    b.ToStream(456, output_b);

    const auto expected =
        "456\n"
        "00:00:01,000 --> 00:00:03,000\n"
        "{\\an1}hello world\n";

    ASSERT_EQ(expected, output_b.str());

    SubRipItem c{b};
    std::ostringstream output_c;
    c.ToStream(456, output_c);

    ASSERT_EQ(expected, output_c.str());
}

TEST(SubRipItemTest, MoveSemantics) {
    SubRipItem a, b;
    b.append_line("should not see");
    a.start(1s)
        ->duration(2s)
        ->position("bottom-left")
        ->append_line("hello world");
    b = std::move(a);

    std::ostringstream output_b;
    b.ToStream(456, output_b);

    const auto expected =
        "456\n"
        "00:00:01,000 --> 00:00:03,000\n"
        "{\\an1}hello world\n";

    ASSERT_EQ(expected, output_b.str());

    SubRipItem c{std::move(b)};
    std::ostringstream output_c;
    c.ToStream(456, output_c);

    ASSERT_EQ(expected, output_c.str());
}

TEST(SubRipItemTest, ConstructFromString_NoBody) {
    std::string subtitles =
        "456\n"
        "00:00:01,000 --> 00:00:03,000\n";

    SubRipItem item{subtitles};
    std::ostringstream output;
    item.ToStream(456, output);

    ASSERT_EQ(subtitles, output.str());
}

TEST(SubRipItemTest, ConstructFromString_TimestampsEqual) {
    std::string subtitles =
        "456\n"
        "00:00:03,000 --> 00:00:03,000\n";

    SubRipItem item{subtitles};
    std::ostringstream output;
    item.ToStream(456, output);

    ASSERT_EQ(subtitles, output.str());
}

TEST(SubRipItemTest, ConstructFromString_WithoutPositionTag) {
    std::string subtitles =
        "456\n"
        "00:00:01,000 --> 00:00:03,000\n"
        "Hello world\n"
        "Goodbye world\n";

    SubRipItem item{subtitles};
    std::ostringstream output;
    item.ToStream(456, output);

    ASSERT_EQ(subtitles, output.str());
}

TEST(SubRipItemTest, ConstructFromString_WithPositionTag) {
    std::string subtitles =
        "456\n"
        "00:00:01,000 --> 00:00:03,000\n"
        "{\\an7}Hello world\n"
        "Goodbye world\n";

    SubRipItem item{subtitles};
    std::ostringstream output;
    item.ToStream(456, output);

    ASSERT_EQ(subtitles, output.str());
}

TEST(SubRipItemTest, ConstructFromString_InvalidSequenceNumThrowsError) {
    std::string subtitles =
        "abc456\n"
        "00:00:01,000 --> 00:00:03,000\n";

    try {
        SubRipItem item{subtitles};
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error &e) {
        ASSERT_STREQ("Could not parse sequence number from: abc456", e.what());
    }
}

TEST(SubRipItemTest, ConstructFromString_InvalidStartTimeThrowsError) {
    std::string subtitles =
        "456\n"
        "00:00:03,000 --> 00:00:02,000\n";

    try {
        SubRipItem item{subtitles};
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error &e) {
        ASSERT_STREQ(
            "Start time cannot be greater than end: 00:00:03,000 --> "
            "00:00:02,000",
            e.what());
    }
}

TEST(SubRipItemTest, ConstructFromString_InvalidPositionIdThrowsError) {
    std::string subtitles =
        "456\n"
        "00:00:01,000 --> 00:00:03,000\n"
        "{\\an10}Hello world\n";

    try {
        SubRipItem item{subtitles};
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error &e) {
        ASSERT_STREQ("Position id must be between [1, 9]: {\\an10}Hello world",
                     e.what());
    }
}

TEST(SubRipItemTest, ConstructFromString_UsingLegacyPositionTagThrowsError) {
    std::string subtitles =
        "456\n"
        "00:00:01,000 --> 00:00:03,000\n"
        "{\\a4}Hello world\n";

    try {
        SubRipItem item{subtitles};
        FAIL() << "Expected std::runtime_error here";
    } catch (const std::runtime_error &e) {
        ASSERT_STREQ("Unsupported position token format: {\\a4}Hello world",
                     e.what());
    }
}
