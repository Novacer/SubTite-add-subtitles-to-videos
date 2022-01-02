#include <iostream>
#include <stdexcept>
#include <memory>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "nlohmann/json.hpp"
#include "date/date.h"

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/play_video/ffplay.h"

DEFINE_string(ffplay_path, "ffplay", "Required. Path to ffplay binary.");
DEFINE_string(ffmpeg_path, "ffmpeg", "Required. Path to ffmpeg binary.");
DEFINE_string(ffprobe_path, "ffprobe", "Required. Path to ffprobe binary.");

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

    using namespace std::chrono_literals;
    using namespace date;
    // format
    auto halfmin = 30.5s;
    std::ostringstream sock;
    to_stream(sock, "%T", halfmin);

    // parse
    std::istringstream pipe{"0:3:15.234"};
    std::chrono::minutes minute;
    from_stream(pipe, "%T", minute);

    LOG(ERROR) << sock.str();
    LOG(ERROR) << minute;

    subtitler::play_video::FFPlay ffplay(
        FLAGS_ffplay_path,
        std::make_unique<subtitler::subprocess::SubprocessExecutor>()
    );

    ffplay.start_pos(2s)
        ->duration(10s)
        ->width(500)
        ->height(500)
        ->OpenPlayer("test.mp4");
    
    std::cin.get(); // simulate delay for user input

    auto captured_error = ffplay.ClosePlayer(1000);
    if (!captured_error.empty()) {
        LOG(ERROR) << captured_error;
    }

    return 0;
}
