#include "subtitler/video/player/ffplay.h"

#include <sstream>
#include <stdexcept>
#include <vector>

#include "date/date.h"
#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/util/font_config.h"
#include "subtitler/video/util/video_utils.h"

namespace subtitler {
namespace video {
namespace player {

namespace {

std::string FormatChrono(const std::chrono::milliseconds &chrono) {
    std::ostringstream stream;
    date::to_stream(stream, "%T", chrono);
    return stream.str();
}

}  // namespace

FFPlay::FFPlay(const std::string &ffplay_path,
               std::unique_ptr<subprocess::SubprocessExecutor> executor)
    : ffplay_path_{ffplay_path},
      executor_{std::move(executor)},
      is_playing_{false} {
    if (ffplay_path_.empty()) {
        throw std::invalid_argument(
            "FFPlay path provided to ffplay cannot be empty!");
    }
    if (!executor_) {
        throw std::invalid_argument(
            "Executor provided to ffplay cannot be null!");
    }
}

FFPlay::~FFPlay() = default;

FFPlay *FFPlay::subtitles_path(const std::string &path) {
    subtitles_path_ = util::FixPathForFilters(path);
    return this;
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
    if (enable_timestamp_ || !subtitles_path_.empty()) {
        // Reference https://stackoverflow.com/questions/67359117
        // Note that if we want to display subtitles also, we must have the
        // filters separated by comma. For example:
        // https://stackoverflow.com/questions/6195872
        args.emplace_back("-vf");
        std::ostringstream draw_text;
        draw_text << '"';
        if (enable_timestamp_) {
            draw_text << "drawtext=text='%{pts\\:hms}':"
                      << "fontsize=(h/30):fontcolor=white:"
                      << "box=1:boxcolor=black";
            auto font_path = get_font_path();
            if (!font_path.empty()) {
                draw_text << ":fontfile='" << get_font_path() << "'";
            }
        }
        if (!subtitles_path_.empty()) {
            if (enable_timestamp_) {
                // draw_text is not empty so we need to prepend comma
                draw_text << ",";
            }
            draw_text << "subtitles='" << subtitles_path_ << "'";
        }
        draw_text << '"';
        args.emplace_back(draw_text.str());
    }
    args.emplace_back("-loglevel error");

    return args;
}

void FFPlay::OpenPlayer(const std::string &video_path) {
    if (video_path.empty()) {
        throw std::invalid_argument("Cannot play empty video path!");
    }
    std::ostringstream command;
    // TODO: always wrap path in quotes.
    command << ffplay_path_ << " " << '"' << video_path << '"';
    auto args = BuildArgs();
    for (const auto &arg : args) {
        command << " " << arg;
    }
    executor_->CaptureOutput(true);
    executor_->SetCommand(command.str());
    executor_->Start();

    is_playing_ = true;
}

std::string FFPlay::ClosePlayer(std::optional<int> timeout_ms) {
    auto ffplay_output = executor_->WaitUntilFinished(timeout_ms);
    is_playing_ = false;
    // Since we set -loglevel error, only error output surfaced to stderr.
    // Not going to throw this, since we almost never want FFPlay error
    // to be a fatal exception.
    return ffplay_output.subproc_stderr;
}

}  // namespace player
}  // namespace video
}  // namespace subtitler
