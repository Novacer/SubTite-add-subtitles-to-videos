#include "subtitler/video/processing/downscaling.h"

#include <cstdint>

namespace subtitler {
namespace video {
namespace processing {

namespace {

// Set to slightly higher than 1080p.
constexpr uint64_t ABOVE_1080P_THRESHOLD = 1950 * 1100;

}  // namespace

std::string GetFFMpegScaleFilterRecommendation(
    const InputVideoScalingInfo input) {
  uint64_t num_pixels =
      static_cast<uint64_t>(input.height) * static_cast<uint64_t>(input.width);

  bool above_1080p = num_pixels > ABOVE_1080P_THRESHOLD;
  // Add one for double epsilon comparison.
  bool above_30fps = input.fps > 31;

  if (above_1080p && above_30fps) {
    // Scale down to SD 480p.
    return "scale=-1:480:flags=lanczos";
  }
  if (above_1080p) {
    // Scale down to 720p internally.
    return "scale=-1:720:flags=lanczos";
  }

  return "";
}

}  // namespace processing
}  // namespace video
}  // namespace subtitler
