#include "subtitler/play_video/ffplay.h"

#include <vector>
#include <sstream>
#include <stdexcept>
#include "date/date.h"
#include "subtitler/subprocess/subprocess_executor.h"

namespace subtitler {
namespace play_video {

namespace {

std::string FormatChrono(const std::chrono::milliseconds& chrono) {
    std::ostringstream stream;
    date::to_stream(stream, "%T", chrono);
    return stream.str();
}

} // namespace

FFPlay::FFPlay(const std::string &ffplay_path, std::unique_ptr<subprocess::SubprocessExecutor> executor):
    ffplay_path_{ffplay_path}, executor_{std::move(executor)} {
    if (!executor_) {
        throw std::invalid_argument("Executor provided to ffplay cannot be null!");
    }
}

std::vector<std::string> FFPlay::BuildArgs() {
    // Consult https://ffmpeg.org/ffplay.html#toc-Main-options
    std::vector<std::string> args;
    if (width_) {
        args.emplace_back("-x");
        args.emplace_back(std::to_string(*width_));
    }
    if (height_) {
        args.emplace_back("-y");
        args.emplace_back(std::to_string(*height_));
    }
    if (fullscreen_) {
        args.emplace_back("-fs");
    }
    if (disable_video_) {
        args.emplace_back("-vn");
    }
    if (disable_audio_) {
        args.emplace_back("-an");
    }
    if (disable_subtitles_) {
        args.emplace_back("-sn");
    }
    if (start_pos_) {
        args.emplace_back("-ss");
        args.emplace_back(FormatChrono(*start_pos_));
    }
    if (duration_) {
        args.emplace_back("-t");
        args.emplace_back(FormatChrono(*duration_));
    }
    if (left_pos_) {
        args.emplace_back("-left");
        args.emplace_back(std::to_string(*left_pos_));
    }
    if (top_pos_) {
        args.emplace_back("-top");
        args.emplace_back(std::to_string(*top_pos_));
    }
    args.emplace_back("-loglevel error");

    return args;
}

void FFPlay::OpenPlayer(const std::string& video_path) {
    if (video_path.empty()) {
        throw std::invalid_argument("Cannot play empty video path!");
    }
    std::ostringstream command;
    command << ffplay_path_ << " " << video_path;
    auto args = BuildArgs();
    for (const auto& arg: args) {
        command << " " << arg;
    }
    executor_->CaptureOutput(true);
    executor_->SetCommand(command.str());
    executor_->Start();
}

std::string FFPlay::ClosePlayer(std::optional<int> timeout_ms) {
    auto ffplay_output = executor_->WaitUntilFinished(timeout_ms);
    if (!ffplay_output.subproc_stderr.empty()) {
        // Since we set -loglevel error, only error output surfaced to stderr.
        // Not going to throw this, since we almost never want FFPlay error
        // to be a fatal exception.
        return ffplay_output.subproc_stderr;
    }
    return std::string{};
}

} // namespace play_video
} // namespace subtitler
