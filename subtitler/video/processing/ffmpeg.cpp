#include "subtitler/video/processing/ffmpeg.h"

#include <sstream>
#include <stdexcept>

#include "subtitler/subprocess/subprocess_executor.h"

namespace subtitler {
namespace video {
namespace processing {

FFMpeg::FFMpeg(const std::string& ffmpeg_path,
               std::unique_ptr<subprocess::SubprocessExecutor> executor)
    : ffmpeg_path_{ffmpeg_path}, executor_{std::move(executor)} {
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
    std::ostringstream stream;
    stream << ffmpeg_path_ << " -version -loglevel error";
    executor_->SetCommand(stream.str());
    executor_->CaptureOutput(true);
    executor_->Start();
    // Give generous 5sec timeout.
    auto output = executor_->WaitUntilFinished(5000);
    // Check for errors
    if (!output.subproc_stderr.empty()) {
        throw std::runtime_error("Error running ffmpeg: " +
                                 output.subproc_stderr);
    }

    return output.subproc_stdout;
}

}  // namespace processing
}  // namespace video
}  // namespace subtitler
