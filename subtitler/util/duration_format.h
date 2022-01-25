#ifndef SUBTITLER_UTIL_UTIL_H
#define SUBTITLER_UTIL_UTIL_H

#include <chrono>
#include <optional>
#include <string>

namespace subtitler {

std::optional<std::chrono::milliseconds> ParseDuration(
    const std::string &duration);

std::string FormatDuration(const std::chrono::milliseconds &duration);

std::optional<std::chrono::milliseconds> FromSubRipDuration(
    std::string duration);

std::string ToSubRipDuration(const std::chrono::milliseconds &duration);

}  // namespace subtitler

#endif
