#include <gflags/gflags.h>
#include <glog/logging.h>

#include <memory>
#include <string>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/processing/ffmpeg.h"
#include "subtitler/video/processing/progress_parser.h"

DEFINE_string(ffmpeg_path, "ffmpeg", "Required. Path to ffmpeg binary.");
DEFINE_string(video_path, "", "Required. Path to the input video.");
DEFINE_string(subtitle_path, "", "Required. Path to the input subtitles.");
DEFINE_string(output_path, "", "Required. Path to the output video.");

namespace {

// Checks that the value of the flag is not empty string.
bool ValidateFlagNonEmpty(const char *flagname, const std::string &value) {
    return !value.empty();
}

// Checks that binaries FFmpeg exists.
void ValidateFFMpeg() {
    subtitler::subprocess::SubprocessExecutor executor;
    executor.CaptureOutput(true);
    executor.SetCommand(FLAGS_ffmpeg_path + " -version");
    executor.Start();
    auto output = executor.WaitUntilFinished(5000);
    if (output.subproc_stdout.empty() || !output.subproc_stderr.empty()) {
        throw std::runtime_error("Error trying to detect binary at " +
                                 FLAGS_ffmpeg_path);
    }
}

}  // namespace

DEFINE_validator(ffmpeg_path, &ValidateFlagNonEmpty);
DEFINE_validator(video_path, &ValidateFlagNonEmpty);
DEFINE_validator(subtitle_path, &ValidateFlagNonEmpty);
DEFINE_validator(output_path, &ValidateFlagNonEmpty);

int main(int argc, char **argv) {
    using namespace subtitler;

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

    auto executor = std::make_unique<subprocess::SubprocessExecutor>();
    video::processing::FFMpeg ffmpeg{FLAGS_ffmpeg_path, std::move(executor)};

    LOG(INFO) << ffmpeg.GetVersionInfo();

    ffmpeg.BurnSubtitlesAsync(
        FLAGS_video_path, FLAGS_subtitle_path, FLAGS_output_path,
        [](const video::processing::Progress &progress) {
            LOG(INFO) << "frame=" << progress.frame << " fps=" << progress.fps;
        });
    ffmpeg.WaitForAsyncTask();
}
