#ifndef SUBTITLER_VIDEO_PLAYER_FFPLAY_H
#define SUBTITLER_VIDEO_PLAYER_FFPLAY_H

#include <string>
#include <optional>
#include <chrono>
#include <memory>
#include <vector>

// Forward declaration
namespace subtitler::subprocess {

class SubprocessExecutor;

} // subtitler::subprocess

namespace subtitler {
namespace video {
namespace player {

/**
 * Wrapper for FFPlay functionality which is used as a video player.
 * Sample usage:
 * FFPlay ffplay("path/to/ffplay", std::make_unique<SubprocessExecutor>());
 * ffplay.width(100)->height(200)->OpenPlayer("path/to/video.mp4");
 * // In the meantime, can do anything like block to get user input
 * cin >> something;
 * // Close the player with 1000ms timeout. If timeout expires, force close it.
 * ffplay.ClosePlayer(1000);
 */
class FFPlay {
public:
    FFPlay(const std::string &ffplay_path, std::unique_ptr<subprocess::SubprocessExecutor> executor);
    ~FFPlay();

    // Opens a video player in a separate process. Does not block.
    void OpenPlayer(const std::string &video_path);

    // Close the video player. Returns any output caught from stderr.
    // Video will be allowed to play for at most timeout_ms before it is closed.
    // (That is, player may be closed before the video finished playing!)
    std::string ClosePlayer(std::optional<int> timeout_ms = std::nullopt);

    // Returns true if OpenPlayer() was successful and there is not a matching ClosePlayer() call.
    // This flag does not reliably convey whether the underlying subprocess is running or not.
    bool is_playing() const { return is_playing_; }

    // FFPlay options. Refer to https://ffmpeg.org/ffplay.html#toc-Main-options.
    FFPlay* width(std::optional<int> width) { width_ = width; return this; }
    FFPlay* height(std::optional<int> height) { height_ = height; return this; }
    FFPlay* fullscreen(bool fullscreen) { fullscreen_ = fullscreen; return this; }

    FFPlay* disable_video(bool disable_video) { disable_video_ = disable_video; return this; }
    FFPlay* disable_audio(bool disable_audio) { disable_audio_ = disable_audio; return this; }
    FFPlay* disable_subtitles(bool disable_subtitles) { disable_subtitles_ = disable_subtitles; return this; }

    FFPlay* start_pos(std::optional<std::chrono::milliseconds> start_pos) { start_pos_ = start_pos; return this; }
    FFPlay* duration(std::optional<std::chrono::milliseconds> duration) { duration_ = duration; return this; }

    FFPlay* left_pos(std::optional<int> left_pos) { left_pos_ = left_pos; return this; }
    FFPlay* top_pos(std::optional<int> top_pos) { top_pos_ = top_pos; return this; }

    // While the video is playing, display the current timestamp.
    FFPlay* enable_timestamp(bool enable_timestamp) { enable_timestamp_ = enable_timestamp; return this; }

    FFPlay* subtitles_path(const std::string &path) { subtitles_path_ = path; return this; }

private:
    std::string ffplay_path_;
    std::unique_ptr<subprocess::SubprocessExecutor> executor_;
    bool is_playing_;

    std::optional<int> width_;
    std::optional<int> height_;
    bool fullscreen_ = false;
    bool disable_video_ = false;
    bool disable_audio_ = false;
    bool disable_subtitles_ = true;
    std::optional<std::chrono::milliseconds> start_pos_;
    std::optional<std::chrono::milliseconds> duration_;
    std::optional<int> left_pos_;
    std::optional<int> top_pos_;
    std::optional<int> volume_;
    bool enable_timestamp_ = false;
    std::string subtitles_path_;

    std::vector<std::string> BuildArgs();
};

} // namespace player
} // namespace video
} // namespace subtitler

#endif
