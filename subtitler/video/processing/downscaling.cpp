#include "subtitler/video/processing/downscaling.h"

#include <sstream>

namespace subtitler {
namespace video {
namespace processing {

namespace {

constexpr uint64_t ABOVE_1080P_THRESHOLD = 1900 * 1000;

}  // namespace

std::string GetFFMpegScaleFilterRecommendation(
    const InputVideoScalingInfo input) {
    uint64_t num_pixels = static_cast<uint64_t>(input.height) *
                          static_cast<uint64_t>(input.width);

    bool above_1080p = num_pixels > ABOVE_1080P_THRESHOLD;
    // Add one for double epsilon comparison.
    bool above_30fps = input.fps > 31;

    std::ostringstream builder;
    if (above_1080p && above_30fps) {
        // Set fps to 30, but only if the video is 1080p or above.
        builder << "fps=30,";
    }
    if (above_1080p) {
        // Scale down to 720p.
        builder << "scale=-1:720:flags=lanczos";
    }

    return builder.str();
}

}  // namespace processing
}  // namespace video
}  // namespace subtitler
