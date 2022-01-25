#include "subtitler/util/duration_format.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>

using namespace std::chrono_literals;
using namespace subtitler;

TEST(DurationFormatTest, RoundTrip_Seconds) {
    std::string seconds = "123";
    std::string with_decimal = "12.345";

    auto parsed = *ParseDuration(seconds);
    auto parsed_with_decimal = *ParseDuration(with_decimal);

    EXPECT_EQ("00:02:03.000", FormatDuration(parsed));
    EXPECT_EQ("00:00:12.345", FormatDuration(parsed_with_decimal));
}

TEST(DurationFormatTest, RoundTrip_Minutes) {
    std::string minutes = "12:34";
    std::string with_decimal = "12:34.567";

    auto parsed = *ParseDuration(minutes);
    auto parsed_with_decimal = *ParseDuration(with_decimal);

    EXPECT_EQ("00:12:34.000", FormatDuration(parsed));
    EXPECT_EQ("00:12:34.567", FormatDuration(parsed_with_decimal));
}

TEST(DurationFormatTest, RoundTrip_Hours) {
    std::string hours = "1:12:34";
    std::string with_decimal = "1:12:34.567";

    auto parsed = *ParseDuration(hours);
    auto parsed_with_decimal = *ParseDuration(with_decimal);

    EXPECT_EQ("01:12:34.000", FormatDuration(parsed));
    EXPECT_EQ("01:12:34.567", FormatDuration(parsed_with_decimal));
}

TEST(DurationFormatTest, RoundTrip_SubRipFormat) {
    std::string hours = "1:12:34";
    std::string with_comma = "1:12:34,567";

    auto parsed = *FromSubRipDuration(hours);
    auto parsed_with_decimal = *FromSubRipDuration(with_comma);

    EXPECT_EQ("01:12:34,000", ToSubRipDuration(parsed));
    EXPECT_EQ("01:12:34,567", ToSubRipDuration(parsed_with_decimal));
}
