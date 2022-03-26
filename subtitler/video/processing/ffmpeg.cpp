#include "subtitler/video/processing/ffmpeg.h"

#include <sstream>
#include <stdexcept>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/util/video_utils.h"

namespace subtitler {
namespace video {
namespace processing {

FFMpeg::FFMpeg(const std::string& ffmpeg_path,
               std::unique_ptr<subprocess::SubprocessExecutor> executor)
    : ffmpeg_path_{ffmpeg_path},
      executor_{std::move(executor)},
      is_running_{false} {
    if (ffmpeg_path_.empty()) {
        throw std::invalid_argument{"FFMPEG Path cannot be empty"};
    }
    if (!executor_) {
        throw std::invalid_argument{
            "Executor provided to FFMPEG cannot be empty"};
    }
}

FFMpeg::~FFMpeg() = default;

std::string FFMpeg::GetVersionInfo() {
    throwIfRunning();

    std::ostringstream stream;
    stream << ffmpeg_path_ << " -version -loglevel error";
    executor_->SetCommand(stream.str());
    executor_->CaptureOutput(true);
    executor_->Start();
    is_running_ = true;
    // Give generous 5sec timeout.
    auto output = executor_->WaitUntilFinished(5000);
    is_running_ = false;
    // Check for errors
    if (!output.subproc_stderr.empty()) {
        throw std::runtime_error("Error running ffmpeg: " +
                                 output.subproc_stderr);
    }

    return output.subproc_stdout;
}

void FFMpeg::BurnSubtitlesAsync(const std::string& video,
                                const std::string& subtitles,
                                const std::string& output) {
    throwIfRunning();

    std::ostringstream stream;
    stream << ffmpeg_path_;
    stream << " -i '" << video << "'";
    stream << " -vf subtitles='" << util::FixPathForFilters(subtitles) << "'";
    stream << " '" << output << "'";
    stream << " -loglevel error -progress pipe:1 -stats_period 5";

    executor_->SetCommand(stream.str());
    executor_->CaptureOutput(true);
    executor_->Start();
    is_running_ = true;
}

void FFMpeg::WaitForAsyncTask(std::optional<int> timeout_ms) {
    if (!is_running_) {
        throw std::runtime_error{
            "FFMpeg is trying to wait when there are no tasks!"};
    }

    auto output = executor_->WaitUntilFinished(timeout_ms);
    is_running_ = false;
    if (!output.subproc_stderr.empty()) {
        throw std::runtime_error{"Error running ffmpeg: " +
                                 output.subproc_stderr};
    }
}

void FFMpeg::throwIfRunning() {
    if (is_running_) {
        throw std::runtime_error{
            "You must call FFMpeg::WaitForAsyncTask() before executing another "
            "task!"};
    }
}

}  // namespace processing
}  // namespace video
}  // namespace subtitler
