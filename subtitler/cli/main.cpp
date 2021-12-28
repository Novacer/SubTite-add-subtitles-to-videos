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

    LOG(INFO) << "ffplay: " << FLAGS_ffplay_path;

    subtitler::subprocess::SubprocessExecutor executor;
    executor.SetCommand(FLAGS_ffplay_path + " -version");
    executor.CaptureStdout(true);
    
    executor.Start();
    auto output = executor.WaitUntilFinished();
    LOG(INFO) << output;

    using namespace std::chrono_literals;
    using namespace date;
    auto halfmin = 30s;
    std::ostringstream sock;
    LOG(ERROR) << halfmin;

    return 0;
}
