#include "subtitler/video/metadata/ffprobe.h"

#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "subtitler/subprocess/subprocess_executor.h"

using json = nlohmann::json;

namespace subtitler {
namespace video {
namespace metadata {

namespace {

std::chrono::milliseconds StringSecondsToMilliseconds(const std::string &seconds) {
    double to_double = std::stod(seconds);
    std::chrono::duration<double> d{to_double};
    return std::chrono::duration_cast<std::chrono::milliseconds>(d);
}

std::unique_ptr<Metadata> ParseVideoMetadata(const std::string &json_output) {
    auto parsed = json::parse(json_output);
    if (!parsed.contains("streams")) {
        throw std::runtime_error("FFProbe could not read streams info");
    }
    auto metadata = std::make_unique<Metadata>();
    for (const auto &stream : parsed["streams"]) {
        if (!stream.contains("codec_type")) {
            throw std::runtime_error("FFProbe stream is missing codec type");
        }
        if (stream["codec_type"] == "audio") {
            if (metadata->audio) {
                throw std::runtime_error("Warning: there is more than one audio stream");
            }
            metadata->audio = AudioStreamInfo{};
            auto &audio_stream_info = metadata->audio;
            audio_stream_info->index = stream.at("index").get<int>();
            audio_stream_info->codec_name = stream.at("codec_name").get<std::string>();
            audio_stream_info->sample_rate = std::stoi(stream.at("sample_rate").get<std::string>());
            audio_stream_info->channels = stream.at("channels").get<int>();
            audio_stream_info->start_time = StringSecondsToMilliseconds(stream.at("start_time").get<std::string>());
            audio_stream_info->duration = StringSecondsToMilliseconds(stream.at("duration").get<std::string>());
        } else if (stream["codec_type"] == "video") {
            if (metadata->video) {
                throw std::runtime_error("Warning: there is more than one audio stream");
            }
            metadata->video = VideoStreamInfo{};
            auto &video_stream_info = metadata->video;
            video_stream_info->index = stream.at("index").get<int>();
            video_stream_info->codec_name = stream.at("codec_name").get<std::string>();
            video_stream_info->codec_tag_string = stream.at("codec_tag_string").get<std::string>();
            video_stream_info->width = stream.at("width").get<int>();
            video_stream_info->height = stream.at("height").get<int>();
            video_stream_info->has_b_frames = stream.at("has_b_frames").get<int>();
            video_stream_info->start_time = StringSecondsToMilliseconds(stream.at("start_time").get<std::string>());
            video_stream_info->duration = StringSecondsToMilliseconds(stream.at("duration").get<std::string>());
        }
    }

    return metadata;
}

} // namespace

FFProbe::FFProbe(const std::string &ffprobe_path, std::unique_ptr<subprocess::SubprocessExecutor> executor)
    : ffprobe_path_{ffprobe_path}, executor_{std::move(executor)} {
    if (ffprobe_path_.empty()) {
        throw std::invalid_argument("FFProbe path provided cannot be empty!");
    }
    if (!executor_) {
        throw std::invalid_argument("Executor provided to ffprobe cannot be null!");
    }
}

FFProbe::~FFProbe() = default;

std::unique_ptr<Metadata> FFProbe::GetVideoMetadata(const std::string &video_path) {
    if (video_path.empty()) {
        throw std::invalid_argument("Video path is empty");
    }

    std::ostringstream command;
    command << ffprobe_path_ << " " << video_path;
    auto args = BuildArgs();
    for (const auto &arg: args) {
        command << " " << arg;
    }
    executor_->CaptureOutput(true);
    executor_->SetCommand(command.str());
    executor_->Start();
    // Give generous 5sec timeout.
    auto output = executor_->WaitUntilFinished(5000);

    // Check for errors
    if (!output.subproc_stderr.empty()) {
        throw std::runtime_error("Error running ffprobe: " + output.subproc_stderr);
    }

    return ParseVideoMetadata(output.subproc_stdout);
}

std::vector<std::string> FFProbe::BuildArgs() {
    std::vector<std::string> args = {
        "-hide_banner",
        "-show_format",
        "-loglevel error",
        "-show_streams",
        "-print_format json=compact=1"
    };

    return args;
}


} // namespace metadata
} // namespace video
} // namespace subtitler
