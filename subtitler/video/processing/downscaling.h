#ifndef SUBTITLER_VIDEO_PROCESSING_DOWNSCALING_H
#define SUBTITLER_VIDEO_PROCESSING_DOWNSCALING_H

#include <string>

/**
 * Implements methods for downscaling videos above 1080p@60fps.
 * In the GUI, such videos tend to be more latent when presented through
 * the OpenGL renderer. As a workaround, we can downscale them via
 * FFMPEG. Roughly, this method allows us to play videos up to ~4k@60fps without
 * much issue.
 *
 */

namespace subtitler {
namespace video {
namespace processing {

// Information about the input video.
struct InputVideoScalingInfo {
    int height;
    int width;
    double fps;
};

std::string GetFFMpegScaleFilterRecommendation(
    const InputVideoScalingInfo input);

}  // namespace processing
}  // namespace video
}  // namespace subtitler

#endif
