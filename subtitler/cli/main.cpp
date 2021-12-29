#include <iostream>
#include <stdexcept>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "nlohmann/json.hpp"
#include "date/date.h"

#include "subtitler/subprocess/subprocess_executor.h"

DEFINE_string(ffplay_path, "", "Required. Path to ffplay binary.");
DEFINE_string(ffmpeg_path, "", "Required. Path to ffmpeg binary.");
DEFINE_string(ffprobe_path, "", "Required. Path to ffprobe binary.");

namespace {

bool ValidateFlagNonEmpty(const char *flagname, const std::string &value) {
    return !value.empty();
}

} // namespace

DEFINE_validator(ffplay_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffmpeg_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffprobe_path, &ValidateFlagNonEmpty);

int main(int argc, char **argv) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

    LOG(ERROR) << "ffplay: " << FLAGS_ffplay_path;

    subtitler::subprocess::SubprocessExecutor executor;
    executor.SetCommand(FLAGS_ffplay_path + " -version");
    executor.CaptureOutput(true);
    
    executor.Start();

    // Simulate main thread being blocked.
    std::cin.get();

    auto output = executor.WaitUntilFinished();
    LOG(ERROR) << output.subproc_stdout;
    LOG(ERROR) << output.subproc_stderr;

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

    return 0;
}
