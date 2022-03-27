#include <gflags/gflags.h>
#include <glog/logging.h>

#include <filesystem>
#include <string>
#include <memory>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/processing/ffmpeg.h"
#include "subtitler/video/processing/progress_parser.h"

DEFINE_string(ffmpeg_path, "ffmpeg", "Required. Path to ffmpeg binary.");

namespace {

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

int main(int argc, char **argv) {
    using namespace subtitler;
    namespace fs = std::filesystem;

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

    std::string video_path =
        "D:/Lecture Videos/CLAS_201 Videos/Week 1ii - Introduction.mp4";
    std::string subtitles = "D:/Lecture Videos/CLAS_201 Videos/test.srt";
    std::string output = "D:/Lecture Videos/CLAS_201 Videos/output.mp4";

    auto executor = std::make_unique<subprocess::SubprocessExecutor>();
    video::processing::FFMpeg ffmpeg{FLAGS_ffmpeg_path, std::move(executor)};

    LOG(INFO) << ffmpeg.GetVersionInfo();
    
    ffmpeg.BurnSubtitlesAsync(video_path, subtitles, output, [](const video::processing::Progress& progress) {
        LOG(INFO) << "frame=" << progress.frame << " fps=" << progress.fps;
    });
    ffmpeg.WaitForAsyncTask();
}
