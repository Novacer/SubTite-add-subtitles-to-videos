#include "subtitler/cli/commands.h"

#include <chrono>
#include <vector>
#include <algorithm>
#include <optional>
#include "date/date.h"
#include "subtitler/play_video/ffplay.h"
#include "subtitler/util/duration_format.h"

namespace subtitler {
namespace cli {

namespace {

const int WAIT_TIMEOUT_MS = 500;
const char *HELP_COMMAND = "help";
const char *PLAY_COMMAND = "play";
const char *DONE_COMMAND = "done";
const char *QUIT_COMMAND = "quit";

std::vector<std::string> Tokenize(const std::string &command) {
    std::vector<std::string> tokens;
    std::istringstream splitter{command};
    std::string token;
    while (splitter >> token) {
        tokens.emplace_back(token);
    }
    return tokens;
}

void CloseAnyOpenPlayers(play_video::FFPlay *ffplay, std::ostream &output) {
    if (ffplay->is_playing()) {
        auto captured_error = ffplay->ClosePlayer(WAIT_TIMEOUT_MS);
        if (!captured_error.empty()) {
            output << "Error closing player: " << captured_error << std::endl;
        }
    }
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

        if (tokens.front() == PLAY_COMMAND) {
            // Handoff remaining tokens to play()
            tokens.erase(tokens.begin());
            Play(tokens);
        } else if (tokens.front() == DONE_COMMAND) {
            Done();
        } else if (tokens.front() == HELP_COMMAND) {
            Help();
        } else if (tokens.front() == QUIT_COMMAND) {
            Quit();
            // Exit the loop.
            break;
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
            << "        12.345 => 12.345 seconds (can have at most 5 digits)" << std::endl
            << "        12:34.0 => 12 minutes 34 seconds" << std::endl
            << "        1:23:45 => 1 hour 23 minutes 45 seconds" << std::endl
            << "        sample usage:" << std::endl
            << "        play start 1:30 duration 5.5 will play video from 1m30s to 1m35.5s" << std::endl;
    output_ << "done -- Saves the current subtitles and moves the position" << std::endl
            << "        to the next 5 seconds of video" << std::endl;
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

    output_ << "Playing start=" << FormatDuration(start_) << " duration=" << FormatDuration(duration_) << std::endl;
    CloseAnyOpenPlayers(ffplay_.get(), output_);

    try {
        ffplay_->start_pos(start_)
            ->duration(duration_)
            ->OpenPlayer(paths_.video_path);
    } catch (const std::exception &e) {
        output_ << "Error opening player: " << e.what() << std::endl;
    }
}

void Commands::Done() {
    CloseAnyOpenPlayers(ffplay_.get(), output_);
    // Commit the currently entered subtitles.
    // Move the current position over
    start_ += duration_;
    duration_ = 5s;
    output_ << "Updated start=" << FormatDuration(start_) << " duration=" << FormatDuration(duration_) << std::endl;
}

void Commands::Quit() {
    CloseAnyOpenPlayers(ffplay_.get(), output_);
    // This is the normal exit.
    // TODO: Could output any srt files here. But not guaranteed to run if exception thrown or user hits Ctrl-C.
}

} // namespace cli
} // namespace subtitler
