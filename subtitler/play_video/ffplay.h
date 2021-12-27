#ifndef _SUBTITLER_PLAY_VIDEO_FFPLAY_H
#define _SUBTITLER_PLAY_VIDEO_FFPLAY_H

#include <string>
#include <optional>
#include <chrono>
#include <memory>

// Forward declaration
namespace subtitler::subprocess {
    class SubprocessExecutor;
} // subtitler::subprocess

namespace subtitler {
namespace play_video {

class FFPlay {
public:
    explicit FFPlay(const std::string &ffplay_path, std::unique_ptr<subprocess::SubprocessExecutor> executor);

    FFPlay* width(std::optional<int> width) { width_ = width; return this; }
    FFPlay* height(std::optional<int> height) { height_ = height; return this; }
    FFPlay* fullscreen(bool fullscreen) { fullscreen_ = fullscreen; return this; }
    FFPlay* disable_video(bool disable_video) { disable_video_ = disable_video; return this; }
    FFPlay* disable_audio(bool disable_audio) { disable_audio_ = disable_audio; return this; }
    FFPlay* disable_subtitles(bool disable_subtitles) { disable_subtitles_ = disable_subtitles; return this; }
    FFPlay* start_pos(std::chrono::milliseconds start_pos) { start_pos_ = start_pos; return this; }
    FFPlay* duration(std::chrono::milliseconds duration) { duration_ = duration; return this; }
    FFPlay* left_pos(std::optional<int> left_pos) { left_pos_ = left_pos; return this; }
    FFPlay* top_pos(std::optional<int> top_pos) { top_pos_ = top_pos; return this; }

private:
    std::optional<int> width_;
    std::optional<int> height_;
    bool fullscreen_ = false;
    bool disable_video_ = false;
    bool disable_audio_ = false;
    bool disable_subtitles_ = true;
    std::chrono::milliseconds start_pos_;
    std::chrono::milliseconds duration_;
    std::optional<int> left_pos_;
    std::optional<int> top_pos_;
    std::optional<int> volume_;
};

} // namespace play_video
} // namespace subtitler

#endif
