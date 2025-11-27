#include "subtitler/video/processing/downscaling.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string_view>

namespace subtitler {
namespace video {
namespace processing {

namespace {

constexpr std::string_view SCALE_FILTER = "scale=-1:720:flags=lanczos";
constexpr std::string_view FPS_SCALE_FILTER = "scale=-1:480:flags=lanczos";

TEST(DownscalingTest, Video_720p_30fps) {
  struct InputVideoScalingInfo input{/*width=*/1280, /*height=*/720,
                                     /*fps=*/30};
  EXPECT_EQ(GetFFMpegScaleFilterRecommendation(input), "");
}

TEST(DownscalingTest, Video_720p_60fps) {
  struct InputVideoScalingInfo input{/*width=*/1280, /*height=*/720,
                                     /*fps=*/60};
  EXPECT_EQ(GetFFMpegScaleFilterRecommendation(input), "");
}

TEST(DownscalingTest, Video_1080p_30fps) {
  struct InputVideoScalingInfo input{/*width=*/1920, /*height=*/1080,
                                     /*fps=*/30};
  EXPECT_EQ(GetFFMpegScaleFilterRecommendation(input), "");
}

TEST(DownscalingTest, Video_1080p_60fps) {
  struct InputVideoScalingInfo input{/*width=*/1920, /*height=*/1080,
                                     /*fps=*/60};
  EXPECT_EQ(GetFFMpegScaleFilterRecommendation(input), "");
}

TEST(DownscalingTest, Video_4K_30fps) {
  struct InputVideoScalingInfo input{/*width=*/3840, /*height=*/2160,
                                     /*fps=*/30};
  EXPECT_EQ(GetFFMpegScaleFilterRecommendation(input), SCALE_FILTER);
}

TEST(DownscalingTest, Video_4K_60fps) {
  struct InputVideoScalingInfo input{/*width=*/3840, /*height=*/2160,
                                     /*fps=*/60};
  EXPECT_EQ(GetFFMpegScaleFilterRecommendation(input), FPS_SCALE_FILTER);
}

}  // namespace

}  // namespace processing
}  // namespace video
}  // namespace subtitler
