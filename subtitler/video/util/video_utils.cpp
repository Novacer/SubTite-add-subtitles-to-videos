#include "subtitler/video/util/video_utils.h"

#include <string_view>

extern "C" {
#include <libavformat/avformat.h>
}

#include <sstream>

namespace subtitler {
namespace video {
namespace util {

std::string FixPathForFilters(const std::string_view path) {
  std::ostringstream output;
  for (const auto& c : path) {
    if (c == '\\') {
      output << '/';
    } else if (c == ':') {
      output << "\\:";
    } else {
      output << c;
    }
  }
  return output.str();
}

std::chrono::microseconds GetVideoDuration(const std::string& video_path) {
  AVFormatContext* pFormatCtx = avformat_alloc_context();
  avformat_open_input(&pFormatCtx, video_path.c_str(), NULL, NULL);
  avformat_find_stream_info(pFormatCtx, NULL);
  auto duration_us = pFormatCtx->duration;
  avformat_close_input(&pFormatCtx);
  avformat_free_context(pFormatCtx);

  return std::chrono::microseconds{duration_us};
}

}  // namespace util
}  // namespace video
}  // namespace subtitler
