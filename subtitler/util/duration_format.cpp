#include "subtitler/util/duration_format.h"

#include <algorithm>
#include <sstream>

#include "date/date.h"

using namespace std::chrono_literals;

namespace subtitler {

std::optional<std::chrono::milliseconds> ParseDuration(
    const std::string& duration) {
  // Verify allowable characters
  for (const auto& c : duration) {
    if (!std::isdigit(c) && c != '.' && c != ':') {
      return std::nullopt;
    }
  }

  // TODO: return nullopt if seconds exceeds 5 digits.
  // Ex: 12345, 123.45, 12.345 etc.

  auto num_colons = std::count(duration.begin(), duration.end(), ':');
  std::istringstream stream{duration};
  std::chrono::milliseconds result = 0ms;
  std::string format = "";

  switch (num_colons) {
    case 0: {
      // Interpret as seconds
      format = "%S";
      break;
    }
    case 1: {
      // Interpret as minutes:seconds
      format = "%M:%S";
      break;
    }
    case 2: {
      // Interpret as hours:minutes:seconds
      format = "%T";
      break;
    }
    default: {
      break;
    }
  }
  if (date::from_stream(stream, format.c_str(), result)) {
    return result;
  }
  return std::nullopt;
}

std::string FormatDuration(const std::chrono::milliseconds& duration) {
  std::ostringstream stream;
  date::to_stream(stream, "%T", duration);
  return stream.str();
}

std::optional<std::chrono::milliseconds> FromSubRipDuration(
    std::string duration) {
  // Replace comma with decimals
  std::replace(duration.begin(), duration.end(), ',', '.');
  return ParseDuration(duration);
}

std::string ToSubRipDuration(const std::chrono::milliseconds& duration) {
  // First convert to standard duration, then replace any decimals with comma.
  // Ex: 00:00:12.340 => 00:00:12,340
  auto result = FormatDuration(duration);
  std::replace(result.begin(), result.end(), '.', ',');
  return result;
}

}  // namespace subtitler
