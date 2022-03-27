#ifndef SUBTITLER_VIDEO_PROCESSING_FFMPEG_H
#define SUBTITLER_VIDEO_PROCESSING_FFMPEG_H

#include <memory>
#include <optional>
#include <string>

// Forward declaration
namespace subtitler::subprocess {

class SubprocessExecutor;

}  // namespace subtitler::subprocess

namespace subtitler {
namespace video {
namespace processing {

/**
 * Wrapper class around FFMPEG for long-running tasks which transform videos.
 */
class FFMpeg {
  public:
    FFMpeg(const std::string& ffmpeg_path,
           std::unique_ptr<subprocess::SubprocessExecutor> executor);
    ~FFMpeg();

    /**
     * Returns the version info from the FFMPEG binary. Useful for debugging.
     *
     * @return std::string the output of `ffmpeg -version`
     */
    std::string GetVersionInfo();

    /**
     * Starts async task to burn subtitles into video, writing result to output.
     * Caller must eventually call WaitForAsyncTask() after calling this.
     * Throws runtime_error if another async task is running at the call.
     *
     * @param video The path of the input video file.
     * @param subtitles The path of the input subtitle (.srt) file.
     * @param output The path of the output file.
     */
    void BurnSubtitlesAsync(const std::string& video,
                            const std::string& subtitles,
                            const std::string& output);

    /**
     * Waits (blocks) for the last async task launched to be completed.
     * Throws runtime_error if no async task is running.
     *
     * @param timeout_ms the amount of time to wait before cancelling the task.
     */
    void WaitForAsyncTask(std::optional<int> timeout_ms = std::nullopt);

  private:
    std::string ffmpeg_path_;
    std::unique_ptr<subprocess::SubprocessExecutor> executor_;
    bool is_running_;

    void throwIfRunning();
};

}  // namespace processing
}  // namespace video
}  // namespace subtitler

#endif
