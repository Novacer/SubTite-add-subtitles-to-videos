#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include "subtitler/srt/subrip_item.h"

using namespace std::chrono_literals;
using namespace subtitler;
using namespace subtitler::srt;

TEST(SubRipItemTest, MultiLineSubtitle) {
    SubRipItem item;
    item.start(1s + 123ms)
        ->duration(5s)
        ->append_line("This is a subtitle!")
        ->append_line("This is another line!");

    std::ostringstream output;
    item.ToStream(123, output);

    ASSERT_EQ(
        "123\n"
        "00:00:01,123 --> 00:00:06,123\n"
        "This is a subtitle!\n"
        "This is another line!\n"
        "\n",
        output.str()
    );
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

    ASSERT_EQ(
        "123\n"
        "00:00:01,123 --> 00:00:06,123\n"
        "This line should be seen!\n"
        "\n",
        output.str()
    );
}

TEST(SubRipItemTest, SetPosition) {
    SubRipItem item;
    item.start(1s + 123ms)
        ->duration(5s)
        ->position("top-left")
        ->append_line("Hello World!")
        ->append_line("Foo bar baz.");

    std::ostringstream output;
    item.ToStream(123, output);

    ASSERT_EQ(
        "123\n"
        "00:00:01,123 --> 00:00:06,123\n"
        "{an7} Hello World!\n"
        "Foo bar baz.\n"
        "\n",
        output.str()
    );
}

TEST(SubRipItemTest, Compare) {
    SubRipItem a, b, c;
    a.start(1s)
        ->duration(4s);
    b.start(2000ms)
        ->duration(1s);
    c.start(1s)
        ->duration(2s);

    ASSERT_LT(a, b);
    ASSERT_LT(c, a);
}
