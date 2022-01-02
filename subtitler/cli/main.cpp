#include <iostream>
#include <stdexcept>
#include <memory>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/play_video/ffplay.h"
#include "subtitler/cli/commands.h"

DEFINE_string(ffplay_path, "ffplay", "Required. Path to ffplay binary.");
DEFINE_string(ffmpeg_path, "ffmpeg", "Required. Path to ffmpeg binary.");
DEFINE_string(ffprobe_path, "ffprobe", "Required. Path to ffprobe binary.");
DEFINE_string(video_path, "", "Optional. Path to the video you want to subtitle. "
                              "If not provided will be asked from stdin.");

namespace {

// Checks that the value of the flag is not empty string.
bool ValidateFlagNonEmpty(const char *flagname, const std::string &value) {
    return !value.empty();
}

// Checks that binaries FFmpeg, FFplay, FFprobe all exist.
void ValidateFFBinaries() {
    subtitler::subprocess::SubprocessExecutor executor;
    executor.CaptureOutput(true);

    for (const auto &binary: {FLAGS_ffplay_path, FLAGS_ffmpeg_path, FLAGS_ffprobe_path}) {
        executor.SetCommand(binary + " -version");
        executor.Start();
        auto output = executor.WaitUntilFinished(5000);
        if (output.subproc_stdout.empty() || !output.subproc_stderr.empty()) {
            throw std::runtime_error("Error trying to detect binary at " + binary);
        }
    }
}

} // namespace

DEFINE_validator(ffplay_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffmpeg_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffprobe_path, &ValidateFlagNonEmpty);

int main(int argc, char **argv) {
    using namespace subtitler;

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

    try {
        ValidateFFBinaries();
    } catch (const std::runtime_error &e) {
        LOG(ERROR) << e.what();
        LOG(ERROR) << "Please verify you have ffmpeg, ffplay, and ffpath installed.";
        LOG(ERROR) << "You can either add these to your PATH variable and the CLI can detect them automatically.";
        LOG(ERROR) << "Or you may provide the paths using --ffmpeg_path, --ffprobe_path, --ffplay_path flags";
        return 1;
    }

    LOG(INFO) << "Successfully detected all binaries!";

    if (FLAGS_video_path.empty()) {
        std::cout << "Please provide path to video you want to subtitle:" << std::endl;
        if (!std::getline(std::cin, FLAGS_video_path)) {
            LOG(ERROR) << "Unable to get video_path!";
            return 1;
        }
    }

    auto executor = std::make_unique<subprocess::SubprocessExecutor>();
    auto ffplay = std::make_unique<play_video::FFPlay>(FLAGS_ffplay_path, std::move(executor));
    cli::Commands::Paths paths{FLAGS_video_path};
    cli::Commands commands{paths, std::move(ffplay), std::cin, std::cout};

    try {
        commands.MainLoop();
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what();
        return 1;
    }
    
    return 0;
}
