#ifndef SUBTITLER_VIDEO_PROCESSING_PROGRESS_PARSER_H
#define SUBTITLER_VIDEO_PROCESSING_PROGRESS_PARSER_H

#include <chrono>
#include <optional>
#include <sstream>

namespace subtitler {
namespace video {
namespace processing {

struct Progress {
    uint32_t frame = 0;
    double fps = 0.0;
    // output stream quantizer not included.
    std::string bitrate;
    uint64_t total_size = 0;
    std::chrono::microseconds out_time_us = std::chrono::microseconds::zero();
    int dup_frames = 0;
    int drop_frames = 0;
    std::string speed;
    std::string progress;
};

class ProgressParser {
  public:
    ProgressParser() = default;
    ~ProgressParser() = default;

    std::optional<Progress> Receive(const char* input);

  private:
    std::ostringstream buffer_;
};

}  // namespace processing
}  // namespace video
}  // namespace subtitler

#endif
