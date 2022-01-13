#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <sstream>
#include "subtitler/srt/subrip_file.h"
#include "subtitler/srt/subrip_item.h"

using namespace std::chrono_literals;
using namespace subtitler;
using namespace subtitler::srt;
using ::testing::UnorderedElementsAre;

class SubRipFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        SubRipItem item;
        item.start(1s)
            ->duration(5s)
            ->append_line("third");
        file.AddItem(item);

        item.start(2s)
            ->duration(5s)
            ->clear_payload()
            ->append_line("fourth");
        file.AddItem(item);

        item.start(1s)
            ->duration(4s)
            ->clear_payload()
            ->append_line("second");
        file.AddItem(item);

        item.start(0s)
            ->duration(20s)
            ->clear_payload()
            ->append_line("first");
        file.AddItem(item);
    }

    SubRipFile file;
};

TEST_F(SubRipFileTest, AddMaintainsSortedOrder) {
    std::ostringstream output;
    file.ToStream(output);
    
    ASSERT_EQ(4, file.NumItems());
    ASSERT_EQ(
        "1\n"
        "00:00:00,000 --> 00:00:20,000\n"
        "first\n"
        "\n"
        "2\n"
        "00:00:01,000 --> 00:00:05,000\n"
        "second\n"
        "\n"
        "3\n"
        "00:00:01,000 --> 00:00:06,000\n"
        "third\n"
        "\n"
        "4\n"
        "00:00:02,000 --> 00:00:07,000\n"
        "fourth\n"
        "\n",
        output.str()
    );
}

TEST_F(SubRipFileTest, FindCollisions) {
    auto collisions = file.GetCollisions(500ms, 1s);
    std::vector<std::string> collided_items_outputs;
    for (const auto& [k, v]: collisions) {
        std::ostringstream output;
        v->ToStream(k, output);
        collided_items_outputs.emplace_back(output.str());
    }

    ASSERT_THAT(collided_items_outputs, UnorderedElementsAre(
        "1\n"
        "00:00:00,000 --> 00:00:20,000\n"
        "first\n",
        "2\n"
        "00:00:01,000 --> 00:00:05,000\n"
        "second\n",
        "3\n"
        "00:00:01,000 --> 00:00:06,000\n"
        "third\n"
    ));
}

TEST_F(SubRipFileTest, AddRemoveThenReadd) {
    SubRipItem item;
    item.start(1s)
        ->duration(5s)
        ->append_line("third replace");
    file.RemoveItem(3);
    file.AddItem(item);

    std::ostringstream output;
    file.ToStream(output);
    
    ASSERT_EQ(4, file.NumItems());
    ASSERT_EQ(
        "1\n"
        "00:00:00,000 --> 00:00:20,000\n"
        "first\n"
        "\n"
        "2\n"
        "00:00:01,000 --> 00:00:05,000\n"
        "second\n"
        "\n"
        "3\n"
        "00:00:01,000 --> 00:00:06,000\n"
        "third replace\n"
        "\n"
        "4\n"
        "00:00:02,000 --> 00:00:07,000\n"
        "fourth\n"
        "\n",
        output.str()
    );
}

TEST_F(SubRipFileTest, ToStreamRangeFilterPreservesSequentialNumbers) {
    std::ostringstream output;
    file.ToStream(output, /* start= */ 6s + 100ms, /* duration= */ 3s);
    
    ASSERT_EQ(
        "1\n"
        "00:00:00,000 --> 00:00:20,000\n"
        "first\n"
        "\n"
        "2\n"
        "00:00:02,000 --> 00:00:07,000\n"
        "fourth\n"
        "\n",
        output.str()
    );
}

TEST_F(SubRipFileTest, ChangePosition) {
    std::ostringstream output;
    file.EditItemPosition(1, "bottom-left");
    file.EditItemPosition(3, "top-right");
    file.ToStream(output);

    ASSERT_EQ(
        "1\n"
        "00:00:00,000 --> 00:00:20,000\n"
        "{\\an1} first\n"
        "\n"
        "2\n"
        "00:00:01,000 --> 00:00:05,000\n"
        "second\n"
        "\n"
        "3\n"
        "00:00:01,000 --> 00:00:06,000\n"
        "{\\an9} third\n"
        "\n"
        "4\n"
        "00:00:02,000 --> 00:00:07,000\n"
        "fourth\n"
        "\n",
        output.str()
    );
}
