#ifndef SUBTITLER_VIDEO_UTIL_VIDEO_UTILS_H
#define SUBTITLER_VIDEO_UTIL_VIDEO_UTILS_H

#include <chrono>
#include <string>

namespace subtitler {
namespace video {
namespace util {

// Replaces backwards slashes with forward slashes.
// Replaces 'C:' with 'C\:'
// Needed to make windows file paths work with ffmpeg style filters.
std::string FixPathForFilters(const std::string &path);

std::chrono::microseconds GetVideoDuration(const std::string &video_path);

}  // namespace util
}  // namespace video
}  // namespace subtitler

#endif
