#include "subtitler/cli/commands.h"

#include <chrono>
#include <vector>
#include <algorithm>
#include <optional>
#include "date/date.h"
#include "subtitler/play_video/ffplay.h"

namespace subtitler {
namespace cli {

namespace {

std::vector<std::string> Tokenize(const std::string &command) {
    std::vector<std::string> tokens;
    std::istringstream splitter{command};
    std::string token;
    while (splitter >> token) {
        tokens.emplace_back(token);
    }
    return tokens;
}

// TODO: consider placing these functions in util module.
std::optional<std::chrono::milliseconds> ParseDuration(const std::string &duration) {
    // Verify allowable characters
    for (const auto &c: duration) {
        if (!std::isdigit(c) && c != '.' && c != ':') {
            return std::nullopt;
        }
    }

    auto num_colons = std::count(duration.begin(), duration.end(), ':');
    std::istringstream stream{duration};
    std::chrono::milliseconds result;
    std::string format = "";

    switch (num_colons) {
        case 0: {
            // Interpret as seconds
            format = "%S";
            break;
        }
        case 1: {
            // Interpret as minutes:seconds
            format = "%M:%S";
            break;
        }
        case 2: {
            // Interpret as hours:minutes:seconds
            format = "%T";
            break;
        }
        default: {
            break;
        }
    }
    if (date::from_stream(stream, format.c_str(), result)) {
        return result;
    }
    return std::nullopt;
}

std::string FormatDuration(const std::chrono::milliseconds &duration) {
    std::ostringstream stream;
    date::to_stream(stream, "%T", duration);
    return stream.str();
}

} // namespace

using namespace std::chrono_literals;
using namespace date;

Commands::Commands(const Paths &paths,
    std::unique_ptr<play_video::FFPlay> ffplay,
    std::istream &input,
    std::ostream &output):
    paths_{paths}, ffplay_{std::move(ffplay)}, input_{input}, output_{output}, start_{0ms}, duration_{5s} {}

Commands::~Commands() = default;

void Commands::MainLoop() {
    output_ << "Starting interactive mode. Type help for instructions." << std::endl;
    output_ << "Initialized with start=" << FormatDuration(start_)
            << " duration=" << FormatDuration(duration_) << std::endl;
    
    std::string command;
    while (std::getline(input_, command)) {
        auto tokens = Tokenize(command);

        if (tokens.empty()) continue;

        if (tokens.front() == "play") {
            // Handoff remaining tokens to play()
            tokens.erase(tokens.begin());
            Play(tokens);
        } else if (tokens.front() == "help") {
            Help();
        } else {
            output_ << "Command " << command << " not recognized!" << std::endl;
        }
    }
}

void Commands::Help() {
    output_ << "Supported commands:" << std::endl;
    output_ << "help -- prints the supported commands" << std::endl;
    output_ << "play -- Play the video at the current position. Use start {time} duration {time} to set" << std::endl
            << "        when to start playing the video and for how long." << std::endl
            << "        {time} can be formatted as:" << std::endl
            << "        123.45 => 123.45 seconds" << std::endl
            << "        12:34.0 => 12 minutes 34 seconds" << std::endl
            << "        1:23:45 => 1 hour 23 minutes 45 seconds" << std::endl
            << "        sample usage:" << std::endl
            << "        play start 1:30 duration 5.5 will play video from 1m30s to 1m35.5s" << std::endl;
}

void Commands::Play(const std::vector<std::string> &tokens) {
    int i = 0;
    // Go through the tokens and update start/duration if needed.
    while (i < tokens.size()) {
        if (tokens.at(i) == "start") {
            if (i + 1 >= tokens.size()) {
                output_ << "Missing start time!" << std::endl;
                return;
            }
            const auto start_opt = ParseDuration(tokens.at(i + 1));
            if (!start_opt) {
                output_ << "Unable to parse start time!" << std::endl;
                return;
            }
            start_ = *start_opt;
            i += 2;
        } else if (tokens.at(i) == "duration") {
            if (i + 1 >= tokens.size()) {
                output_ << "Missing duration time!" << std::endl;
                return;
            }
            const auto duration_opt = ParseDuration(tokens.at(i + 1));
            if (!duration_opt) {
                output_ << "Unable to parse duration time!" << std::endl;
                return;
            }
            duration_ = *duration_opt;
            i += 2;
        } else {
            output_ << "Unrecognized token: " << tokens.at(i) << std::endl;
            return;
        }
    }

    output_ << "Using start=" << FormatDuration(start_) << " duration=" << FormatDuration(duration_) << std::endl;
    try {
        ffplay_->start_pos(start_)
            ->duration(duration_)
            ->OpenPlayer(paths_.video_path);
    } catch (const std::exception &e) {
        output_ << "Error: " << e.what() << std::endl;
    }
}

} // namespace cli
} // namespace subtitler
