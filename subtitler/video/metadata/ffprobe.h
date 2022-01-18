#ifndef SUBTITLER_VIDEO_METADATA_FFPROBE
#define SUBTITLER_VIDEO_METADATA_FFPROBE

#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <chrono>

// Forward declaration
namespace subtitler::subprocess {

class SubprocessExecutor;

} // namespace subtitler::subprocess

namespace subtitler {
namespace video {
namespace metadata {


struct AudioStreamInfo {
    int index;
    std::string codec_name;
    int sample_rate;
    int channels;
    std::chrono::milliseconds start_time;
    std::chrono::milliseconds duration;
};

struct VideoStreamInfo {
    int index;
    std::string codec_name;
    std::string codec_tag_string;
    int width;
    int height;
    int has_b_frames;
    std::chrono::milliseconds start_time;
    std::chrono::milliseconds duration;
};

struct Metadata {
    std::optional<AudioStreamInfo> audio;
    std::optional<VideoStreamInfo> video;
    // TODO: return subtitle streams.
};

class FFProbe {
public:
    FFProbe(const std::string &ffprobe_path, std::unique_ptr<subprocess::SubprocessExecutor> executor);
    ~FFProbe();

    std::unique_ptr<Metadata> GetVideoMetadata(const std::string &video_path);

private:
    std::string ffprobe_path_;
    std::unique_ptr<subprocess::SubprocessExecutor> executor_;

    std::vector<std::string> BuildArgs();
};

} // namespace metadata
} // namespace video
} // namespace subtitler


#endif
