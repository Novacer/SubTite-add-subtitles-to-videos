#include "subtitler/video/processing/ffmpeg.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/processing/progress_parser.h"
#include "subtitler/video/util/video_utils.h"

namespace fs = std::filesystem;

namespace subtitler {
namespace video {
namespace processing {

FFMpeg::FFMpeg(const std::string_view ffmpeg_path,
               std::unique_ptr<subprocess::SubprocessExecutor> executor)
    : ffmpeg_path_{ffmpeg_path},
      executor_{std::move(executor)},
      is_running_{false},
      progress_parser_{nullptr} {
  if (ffmpeg_path_.empty()) {
    throw std::invalid_argument{"FFMPEG Path cannot be empty"};
  }
  if (!executor_) {
    throw std::invalid_argument{"Executor provided to FFMPEG cannot be empty"};
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
    throw std::runtime_error("Error running ffmpeg: " + output.subproc_stderr);
  }

  return output.subproc_stdout;
}

void FFMpeg::ExtractUncompressedAudio(const std::string_view input_video_path,
                                      const std::string_view output_wav_path) {
  throwIfRunning();

  std::ostringstream stream;
  stream << ffmpeg_path_;
  stream << " -y -i " << '"' << input_video_path << '"';
  stream << " " << '"' << output_wav_path << '"';
  stream << " -loglevel error";

  executor_->SetCommand(stream.str());
  executor_->CaptureOutput(true);
  executor_->Start();
  is_running_ = true;
  auto output = executor_->WaitUntilFinished();
  is_running_ = false;
  if (!output.subproc_stderr.empty()) {
    throw std::runtime_error{"Error running ffmpeg: " + output.subproc_stderr};
  }
}

void FFMpeg::RemuxSubtitlesAsync(
    const std::string_view video, const std::string_view subtitles,
    const std::string_view output,
    std::function<void(const Progress&)> progress_callback) {
  throwIfRunning();

  std::ostringstream stream;
  stream << ffmpeg_path_;
  stream << " -y -i " << '"' << video << '"';
  stream << " -i " << '"' << subtitles << '"';
  stream << " -map 0 -map 1:s -c copy";
  stream << " " << '"' << output << '"';
  stream << " -loglevel error -progress pipe:1 -stats_period 5";

  executor_->SetCommand(stream.str());
  executor_->CaptureOutput(false);

  progress_parser_ = std::make_unique<ProgressParser>();
  executor_->SetCallback(
      [this, pcb = std::move(progress_callback)](const char* buffer) {
        const auto progress = progress_parser_->Receive(buffer);
        if (progress) {
          pcb(*progress);
        }
      });
  executor_->Start();
  is_running_ = true;
}

void FFMpeg::BurnSubtitlesAsync(
    const std::string_view video, const std::string_view subtitles,
    const std::string_view output,
    std::function<void(const Progress&)> progress_callback) {
  throwIfRunning();

  std::ostringstream stream;
  stream << ffmpeg_path_;
  stream << " -y -i " << '"' << video << '"';
  stream << " -vf"
         << " \"subtitles='" << util::FixPathForFilters(subtitles) << "'"
         << '"';
  stream << " " << '"' << output << '"';
  stream << " -loglevel error -progress pipe:1 -stats_period 5";

  executor_->SetCommand(stream.str());
  executor_->CaptureOutput(false);

  progress_parser_ = std::make_unique<ProgressParser>();
  executor_->SetCallback(
      [this, pcb = std::move(progress_callback)](const char* buffer) {
        const auto progress = progress_parser_->Receive(buffer);
        if (progress) {
          pcb(*progress);
        }
      });
  executor_->Start();
  is_running_ = true;
}

void FFMpeg::WaitForAsyncTask(std::optional<int> timeout_ms) {
  if (!is_running_) {
    throw std::runtime_error{
        "FFMpeg is trying to wait when there are no tasks!"};
  }

  auto output = executor_->WaitUntilFinished(timeout_ms);
  progress_parser_.reset();
  is_running_ = false;
  if (!output.subproc_stderr.empty()) {
    throw std::runtime_error{"Error running ffmpeg: " + output.subproc_stderr};
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
