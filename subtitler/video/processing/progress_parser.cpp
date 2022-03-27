#include "subtitler/video/processing/progress_parser.h"

#include <algorithm>
#include <array>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace subtitler {
namespace video {
namespace processing {

namespace {

void throwMissingKey(const std::string& key_missing) {
    throw std::runtime_error{"Missing key: " + key_missing};
}

std::array<std::string, 2> split2(const std::string& s, char delim) {
    std::array<std::string, 2> result;
    std::istringstream splitter{s};
    std::string item;

    for (auto& x : result) {
        if (!std::getline(splitter, item, delim)) {
            throw std::runtime_error{"Expected line in key=val form but got: " +
                                     s};
        }
        x = item;
    }

    return result;
}

Progress ParseProgress(const std::string& str) {
    if (str.empty()) {
        throw std::runtime_error{"Cannot parse empty progress string"};
    }

    Progress progress;
    std::istringstream line_splitter{str};
    std::string line;
    if (!std::getline(line_splitter, line) || line.rfind("frame=", 0) != 0) {
        throwMissingKey("frame");
    }
    progress.frame = std::stoul(split2(line, '=').at(1));

    if (!std::getline(line_splitter, line) || line.rfind("fps=", 0) != 0) {
        throwMissingKey("fps");
    }
    progress.fps = std::stod(split2(line, '=').at(1));

    if (!std::getline(line_splitter, line) || line.rfind("stream_", 0) != 0) {
        throwMissingKey("stream_");
    }
    // Skip output stream quantizer.

    if (!std::getline(line_splitter, line) || line.rfind("bitrate=", 0) != 0) {
        throwMissingKey("bitrate");
    }
    progress.bitrate = split2(line, '=').at(1);

    if (!std::getline(line_splitter, line) ||
        line.rfind("total_size=", 0) != 0) {
        throwMissingKey("total_size");
    }
    progress.total_size = std::stoull(split2(line, '=').at(1));

    if (!std::getline(line_splitter, line) ||
        line.rfind("out_time_us=", 0) != 0) {
        throwMissingKey("out_time_us");
    }
    progress.out_time_us =
        std::chrono::microseconds{std::stoull(split2(line, '=').at(1))};

    if (!std::getline(line_splitter, line) ||
        line.rfind("out_time_ms=", 0) != 0) {
        throwMissingKey("out_time_ms");
    }
    // Skip out_time_ms, since it is same as out_time_us.
    // https://ffmpeg.org/pipermail/ffmpeg-user/2016-July/032897.html

    if (!std::getline(line_splitter, line) || line.rfind("out_time=", 0) != 0) {
        throwMissingKey("out_time");
    }
    // Skip out_time, since we can do our own formatting from out_time_us.
    if (!std::getline(line_splitter, line) ||
        line.rfind("dup_frames=", 0) != 0) {
        throwMissingKey("dup_frames");
    }
    progress.dup_frames = std::stoi(split2(line, '=').at(1));

    if (!std::getline(line_splitter, line) ||
        line.rfind("drop_frames=", 0) != 0) {
        throwMissingKey("drop_frames");
    }
    progress.drop_frames = std::stoi(split2(line, '=').at(1));

    if (!std::getline(line_splitter, line) || line.rfind("speed=", 0) != 0) {
        throwMissingKey("speed");
    }
    progress.speed = split2(line, '=').at(1);

    if (!std::getline(line_splitter, line) || line.rfind("progress=", 0) != 0) {
        throwMissingKey("progress");
    }
    progress.progress = split2(line, '=').at(1);

    return progress;
}

}  // namespace

std::optional<Progress> ProgressParser::Receive(const char* input) {
    std::ostringstream line_buffer;
    std::optional<Progress> result = std::nullopt;
    for (const char* x = input; x && *x != '\0'; ++x) {
        line_buffer << *x;
        if (*x == '\n') {
            std::string line = line_buffer.str();
            line_buffer = std::ostringstream{};

            if (std::all_of(line.begin(), line.end(), isspace)) {
                continue;
            }

            buffer_ << line;
            if (line.rfind("progress=", 0) == 0) {
                result = ParseProgress(buffer_.str());
                buffer_ = std::ostringstream{};
                // Continue parsing. Possibly we've recieved a new
                // more up to date progress, in which case we want to
                // return that one instead.
            }
        }
    }
    if (std::string line = line_buffer.str(); !line.empty()) {
        buffer_ << line;
    }

    return result;
}

}  // namespace processing
}  // namespace video
}  // namespace subtitler
