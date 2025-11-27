#ifndef SUBTITLER_VIDEO_PROCESSING_PROGRESS_PARSER_H
#define SUBTITLER_VIDEO_PROCESSING_PROGRESS_PARSER_H

#include <chrono>
#include <optional>
#include <sstream>

namespace subtitler {
namespace video {
namespace processing {

/**
 * Struct containing progress info while running FFMPEG.
 */
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

/**
 * Parses FFMPEG progress and returns as a Progress struct.
 * Data from FFMPEG can be received in chunks. If an incomplete progress
 * update is received, then this parser will buffer the data until
 * a complete update can be returned.
 *
 * If multiple updates are received at once, then this returns the most
 * recent update.
 */
class ProgressParser {
 public:
  ProgressParser() = default;
  ~ProgressParser() = default;

  /**
   * Receive progress data from FFMPEG and return the most recent update.
   *
   * @param input the progress data from FFMPEG.
   * @return std::optional<Progress> Will be nullopt if not enough data
   * received to give an update. Otherwise, returns the most recent update.
   */
  std::optional<Progress> Receive(const char* input);

 private:
  std::ostringstream buffer_;
};

}  // namespace processing
}  // namespace video
}  // namespace subtitler

#endif
