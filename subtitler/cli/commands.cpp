#include "subtitler/cli/commands.h"

#include <chrono>
#include <vector>
#include <algorithm>
#include <optional>
#include <fstream>
#include "date/date.h"
#include "subtitler/play_video/ffplay.h"
#include "subtitler/util/duration_format.h"
#include "subtitler/srt/subrip_item.h"

namespace subtitler {
namespace cli {

namespace {

const int WAIT_TIMEOUT_MS = 100;
const char *HELP_COMMAND = "help";

const char *PLAY_COMMAND = "play";
const char *DONE_COMMAND = "done";

const char *PRINT_SUBS_COMMAND = "printsubs";
const char *ADD_SUB_COMMAND = "addsub";
const char *DELETE_SUB_COMMAND = "deletesub";
const char *SAVE_COMMAND = "save";

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
    paths_{paths},
    ffplay_{std::move(ffplay)},
    input_{input},
    output_{output},
    start_{0ms},
    duration_{5s},
    srt_file_{},
    srt_file_has_changed_{false} {}

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
        } else if (tokens.front() == PRINT_SUBS_COMMAND) {
            PrintSubs();
        } else if (tokens.front() == ADD_SUB_COMMAND) {
            tokens.erase(tokens.begin());
            AddSub(tokens);
        } else if (tokens.front() == DELETE_SUB_COMMAND) {
            tokens.erase(tokens.begin());
            DeleteSub(tokens);
        } else if (tokens.front() == SAVE_COMMAND) {
            Save();
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
    output_ << "help -- Prints the supported commands." << std::endl;
    output_ << "play -- Play the video at the current position." << std::endl
            << "        Use start or s {time} duration or d {time} to set" << std::endl
            << "        when to start playing the video and for how long." << std::endl
            << "        {time} can be formatted as:" << std::endl
            << "        12.345 => 12.345 seconds (can have at most 5 digits)" << std::endl
            << "        12:34.0 => 12 minutes 34 seconds" << std::endl
            << "        1:23:45 => 1 hour 23 minutes 45 seconds" << std::endl
            << "        sample usage:" << std::endl
            << "        play start 1:30 duration 5.5 will play video from 1m30s to 1m35.5s" << std::endl;
    output_ << "done -- Saves the current subtitles and moves the position" << std::endl
            << "        to the next 5 seconds of video" << std::endl;
    output_ << "quit -- Optionally saves and exits." << std::endl;
    output_ << std::endl;
    output_ << "printsubs -- Prints the all subtitles at the current player position." << std::endl;
    output_ << "addsub    -- Adds a subtitle block. Use position or p {position} to set the position." << std::endl
            << "             valid position are: bottom-left, middle-center, top-right and so on." << std::endl
            << "             It is possible to add multiple subtitles to the same section of video." << std::endl;
    output_ << "deletesub -- Removes the subtitle. Use deletesub {seq_num} to identify the subtitle." << std::endl
            << "             Normally, you can only delete subtitles at the current player position." << std::endl
            << "             However deletesub --force {seq_num} enables deleting any subtitle." << std::endl;
    output_ << "save      -- Saves the current SRT to the output file." << std::endl;
}

void Commands::Play(const std::vector<std::string> &tokens) {
    std::size_t i = 0;
    // Go through the tokens and update start/duration if needed.
    while (i < tokens.size()) {
        if (tokens.at(i) == "start" || tokens.at(i) == "s") {
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
        } else if (tokens.at(i) == "duration" || tokens.at(i) == "d") {
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

void Commands::PrintSubs() {
    auto collisions = srt_file_.GetCollisions(start_, duration_);
    for (const auto &[seq_num, item]: collisions) {
        item->ToStream(seq_num, output_);
        output_ << std::endl;
    }
}

void Commands::AddSub(const std::vector<std::string> tokens) {
    std::size_t i = 0;
    srt::SubRipItem item;
    item.start(start_)
        ->duration(duration_);

    while (i < tokens.size()) {
        if (tokens.at(i) == "position" || tokens.at(i) == "p") {
            if (i + 1 >= tokens.size()) {
                output_ << "Missing position id. Valid positions are: " << std::endl;
                output_ << item.substation_alpha_positions << std::endl;
                return;
            }
            try {
                item.position(tokens.at(i + 1));
            } catch (const std::out_of_range &e) {
                output_ << "Position " << tokens.at(i + 1) << " not recognized. Valid positions are: " << std::endl;
                output_ << item.substation_alpha_positions << std::endl;
                return;
            }

            i += 2;
        } else {
            output_ << "Unrecognized token: " << tokens.at(i);
            return;
        }
    }

    output_ << "Enter the subtitles, multiple lines allowed. A blank line (enter) represents end of input." 
            << std::endl;
    
    std::string subtitle;
    while (std::getline(input_, subtitle)) {
        if (subtitle.empty()) {
            break;
        }
        item.append_line(subtitle);
    }
    srt_file_.AddItem(std::move(item));
    srt_file_has_changed_ = true;

    output_ << "Enter next command:" << std::endl;
}

void Commands::DeleteSub(const std::vector<std::string> tokens) {
    std::size_t i = 0;
    const auto collisions = srt_file_.GetCollisions(start_, duration_);
    bool force = false;
    std::size_t sequence_num = 0;
    bool sequence_num_set = false;

    while (i < tokens.size()) {
        if (tokens.at(i) == "--force") {
            force = true;
        } else if (std::istringstream stream{tokens.at(i)}; stream >> sequence_num) {
            if (sequence_num_set) {
                output_ << "Can only provide one sequence number to delete!" << std::endl;
                return;
            }
            sequence_num_set = true;
        } else {
            output_ << "Unrecognized token: " << tokens.at(i) << std::endl;
            return;
        }

        i += 1;
    }
    if (!sequence_num_set) {
        output_ << "Missing sequence num. Check help for usage." << std::endl;
        return;
    }
    if (auto it = collisions.find(sequence_num); it == collisions.end() && !force) {
        output_ << "The subtitle you want to delete is not within the current player position." << std::endl;
        output_ << "Either change position using play or use deletesub --force {sequence_num}" << std::endl;
        return;
    }
    
    try {
        auto deleted_item = srt_file_.RemoveItem(sequence_num);
        output_ << "Deleted: ";
        deleted_item.ToStream(sequence_num, output_);
        output_ << std::endl;
    } catch (const std::out_of_range &e) {
        output_ << "Error while deleting sequence num: " << sequence_num;
        return;
    }

    srt_file_has_changed_ = true;
}

void Commands::Save() {
    std::ofstream file{paths_.output_subtitle_path, std::ofstream::out | std::ofstream::trunc};
    if (!file) {
        throw std::runtime_error("Could not open file " + paths_.output_subtitle_path);
    }
    srt_file_.ToStream(file);
    srt_file_has_changed_ = false;
    output_ << "Saved!" << std::endl;
}

void Commands::Quit() {
    CloseAnyOpenPlayers(ffplay_.get(), output_);
    if (srt_file_has_changed_) {
        output_ << "Save before closing? Input: [Y/n]" << std::endl;
        std::string response;
        std::getline(input_, response);
        // Tokenize to remove whitespace
        auto tokens = Tokenize(response);
        if (tokens.empty() || tokens.front() == "Y" || tokens.front() == "y") {
            // If nothing provided then still prefer to Save().
            Save();
        }
    }
}

} // namespace cli
} // namespace subtitler
