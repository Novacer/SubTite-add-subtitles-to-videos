#ifndef SUBTITLER_VIDEO_PROCESSING_FFMPEG_H
#define SUBTITLER_VIDEO_PROCESSING_FFMPEG_H

#include <memory>
#include <string>

// Forward declaration
namespace subtitler::subprocess {

class SubprocessExecutor;

}  // namespace subtitler::subprocess

namespace subtitler {
namespace video {
namespace processing {

class FFMpeg {
  public:
    FFMpeg(const std::string& ffmpeg_path,
           std::unique_ptr<subprocess::SubprocessExecutor> executor);
    ~FFMpeg();
};

}  // namespace processing
}  // namespace video
}  // namespace subtitler

#endif
