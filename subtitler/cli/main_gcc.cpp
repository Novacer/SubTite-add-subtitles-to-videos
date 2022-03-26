#include <gflags/gflags.h>
#include <glog/logging.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "subtitler/cli/commands.h"
#include "subtitler/cli/io/input.h"
#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/metadata/ffprobe.h"
#include "subtitler/video/player/ffplay.h"

DEFINE_string(ffplay_path, "ffplay", "Required. Path to ffplay binary.");
DEFINE_string(ffmpeg_path, "ffmpeg", "Required. Path to ffmpeg binary.");
DEFINE_string(ffprobe_path, "ffprobe", "Required. Path to ffprobe binary.");
DEFINE_string(video_path, "",
              "Optional. Path to the video you want to subtitle. "
              "If not provided will be asked from stdin.");
DEFINE_string(output_subtitle_path, "",
              "Optional. Path to the output srt file. "
              "If not provided will be asked from stdin.");

namespace {

// Checks that the value of the flag is not empty string.
bool ValidateFlagNonEmpty(const char *flagname, const std::string &value) {
    return !value.empty();
}

// Checks that binaries FFmpeg, FFplay, FFprobe all exist.
void ValidateFFBinaries() {
    using subtitler::subprocess::SubprocessExecutor;
    std::vector<std::unique_ptr<SubprocessExecutor>> executors;
    std::vector<std::string> binaries = {FLAGS_ffplay_path, FLAGS_ffmpeg_path,
                                         FLAGS_ffprobe_path};
    executors.reserve(3);

    for (const auto &binary : binaries) {
        auto &executor = executors.emplace_back(
            std::make_unique<SubprocessExecutor>(binary + " -version",
                                                 /* capture_output= */ true));
        executor->Start();
    }
    std::ostringstream errors;
    for (std::size_t i = 0; i < executors.size(); ++i) {
        auto &executor = executors.at(i);
        auto output = executor->WaitUntilFinished(2000);
        if (output.subproc_stdout.empty() || !output.subproc_stderr.empty()) {
            errors << " " << binaries.at(i);
        }
    }
    auto binary_error_list = errors.str();
    if (!binary_error_list.empty()) {
        throw std::runtime_error("Failed to detect:" + binary_error_list);
    }
}

// Make a best effort to fix otherwise valid input paths.
// For example, in some cases we need quotes when passing input paths like
// "path/to some/folder" as a command line argument. While in some other cases
// (opening file directly) we don't want to use quotes.
void FixInputPath(std::string &path, bool should_have_quotes) {
    if (path.empty() || path.length() < 2) {
        return;
    }
    bool already_in_quotes = (path.front() == '"' && path.back() == '"') ||
                             (path.front() == '\'' && path.back() == '\'');
    if (should_have_quotes && !already_in_quotes) {
        // Any input paths with a space in them needs to be wrapped in quotes.
        bool contains_space = path.find(' ') != std::string::npos;
        if (contains_space) {
            std::ostringstream stream;
            stream << '"' << path << '"';
            path = stream.str();
        }
    } else if (!should_have_quotes && already_in_quotes) {
        // Have to strip the quotes.
        path = path.substr(1, path.length() - 2);
    }
}

}  // namespace

DEFINE_validator(ffplay_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffmpeg_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffprobe_path, &ValidateFlagNonEmpty);

int main(int argc, char **argv) {
    using namespace subtitler;

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

    // If any binary path has spaces, let's make sure they are not
    // interpreted wrongly by wrapping them up with quotes.
    FixInputPath(FLAGS_ffplay_path, /* should_have_quotes= */ true);
    FixInputPath(FLAGS_ffmpeg_path, /* should_have_quotes= */ true);
    FixInputPath(FLAGS_ffprobe_path, /* should_have_quotes= */ true);

    try {
        ValidateFFBinaries();
    } catch (const std::runtime_error &e) {
        LOG(ERROR) << e.what();
        LOG(ERROR)
            << "Please verify you have ffmpeg, ffplay, and ffpath installed.";
        LOG(ERROR) << "You can either add these to your PATH variable and the "
                      "CLI can detect them automatically.";
        LOG(ERROR) << "Or you may provide the paths using --ffmpeg_path, "
                      "--ffprobe_path, --ffplay_path flags";
        return 1;
    }

    LOG(INFO) << "Successfully detected all binaries!";

    if (FLAGS_video_path.empty()) {
        std::cout << "Please provide path to video you want to subtitle:"
                  << std::endl;
        if (!std::getline(std::cin, FLAGS_video_path)) {
            LOG(ERROR) << "Unable to get video_path!";
            return 1;
        }
    }
    if (FLAGS_output_subtitle_path.empty()) {
        std::cout
            << "Please provide path to the file you want to save the subtitles:"
            << std::endl;
        if (!std::getline(std::cin, FLAGS_output_subtitle_path)) {
            LOG(ERROR) << "Unable to get output_subtitle_path!";
            return 1;
        }
    }

    // Make sure input file path is unwrapped with quotes in case
    // user included them.
    FixInputPath(FLAGS_video_path, /* should_have_quotes= */ false);
    FixInputPath(FLAGS_output_subtitle_path, /* should_have_quotes= */ false);

    // Sanity test writing to output path beforehand so we know it works.
    {
        // Write empty string in append mode.
        std::ofstream ofs{FLAGS_output_subtitle_path, std::ios_base::app};
        if (!ofs) {
            LOG(ERROR) << "Unable to open file: " << FLAGS_output_subtitle_path;
            return 1;
        }
        ofs << "";
    }

    // Get video metadata.
    std::unique_ptr<video::metadata::Metadata> video_metadata;
    try {
        video::metadata::FFProbe ffprobe{
            FLAGS_ffprobe_path,
            std::make_unique<subprocess::SubprocessExecutor>()};
        video_metadata = ffprobe.GetVideoMetadata(FLAGS_video_path);
    } catch (const std::exception &e) {
        LOG(INFO) << e.what();
        LOG(ERROR) << "Unable to read video file: " << FLAGS_video_path;
        return 1;
    }

    if (video_metadata == nullptr ||
        !(video_metadata->audio || video_metadata->video)) {
        LOG(ERROR) << "The inputted video has unsupported format / "
                      "It does not have any audio/video tracks";
        return 1;
    }

    cli::Commands::Paths paths{FLAGS_video_path, FLAGS_output_subtitle_path};
    auto ffplay = std::make_unique<video::player::FFPlay>(
        FLAGS_ffplay_path, std::make_unique<subprocess::SubprocessExecutor>());
    auto narrow_input_getter =
        std::make_unique<cli::io::NarrowInputGetter>(std::cin);

    cli::Commands commands{paths, std::move(ffplay),
                           std::move(narrow_input_getter), std::cout,
                           std::move(video_metadata)};

    try {
        commands.MainLoop();
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what();
        return 1;
    }

    return 0;
}
