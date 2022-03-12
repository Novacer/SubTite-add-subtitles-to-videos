#include "subtitler/srt/subrip_item.h"

#include <array>
#include <regex>
#include <string>
#include <tuple>

#include "subtitler/util/duration_format.h"

namespace subtitler {
namespace srt {

namespace {

// Parses sequence number from a string containing a single line.
int ParseSequenceNumber(const std::string &line) {
    try {
        return std::stoi(line);
    } catch (const std::exception &e) {
        throw std::runtime_error{"Could not parse sequence number from: " +
                                 line};
    }
}

// Parses start & end timestamps.
std::tuple<std::chrono::milliseconds, std::chrono::milliseconds>
ParseTimestamps(const std::string &line) {
    std::istringstream stream{line};
    // Expect 3 tokens separated by whitespace
    std::array<std::string, 3> tokens;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (!(stream >> tokens.at(i))) {
            throw std::runtime_error{"Could not parse timestamp line: " + line};
        }
    }
    // Validate middle token is "-->"
    if (tokens.at(1) != "-->") {
        throw std::runtime_error{"Expected \"-->\" but got: " + tokens.at(1)};
    }
    auto start = FromSubRipDuration(tokens.at(0));
    auto end = FromSubRipDuration(tokens.at(2));
    if (!start || !end) {
        throw std::runtime_error{"Could not parse timestamp values: " + line};
    }
    if (*start > *end) {
        throw std::runtime_error{"Start time cannot be greater than end: " +
                                 line};
    }
    return std::make_tuple(*start, *end - *start);
}

// Should be called on the FIRST line of the subtitle body.
// If line is "{\anX}Hello World!" then returns X
// and modifies line to be "Hello World!"
std::optional<int> ExtractPosIdIfExists(std::string &line) {
    static const std::regex pos_token_matcher{"^\\{\\\\an([0-9]+)\\}(.*)"};
    std::smatch matchings;
    bool did_match = std::regex_match(line, matchings, pos_token_matcher);
    bool might_use_legacy_position_token = line.rfind("{\\a", 0) == 0;
    if (!did_match && might_use_legacy_position_token) {
        throw std::runtime_error{"Unsupported position token format: " + line};
    }
    if (!did_match) {
        return std::nullopt;
    }
    if (matchings.size() != 3) {
        throw std::runtime_error("Regex matcher returned wrong size");
    }
    int pos_id = std::stoi(matchings[1].str());
    if (pos_id < 1 || pos_id > 9) {
        throw std::runtime_error{"Position id must be between [1, 9]: " + line};
    }
    line = matchings[2].str();
    return pos_id;
}

}  // namespace

SubRipItem::SubRipItem(const std::string &payload) {
    std::istringstream stream{payload};
    // First line is the sequence number.
    // This is required to be present but ignored,
    // since in the context of the entire file we will reorder
    // the sequence numbers.
    std::string line;
    if (!std::getline(stream, line)) {
        throw std::runtime_error{"Sequence number was missing."};
    }
    ParseSequenceNumber(line);
    // Next line is the timestamps
    if (!std::getline(stream, line)) {
        throw std::runtime_error{"Timestamps were missing."};
    }
    auto [start, duration] = ParseTimestamps(line);
    start_ = start;
    duration_ = duration;

    num_lines_ = 0;
    while (std::getline(stream, line)) {
        // Add subtitle body
        if (num_lines_ == 0) {
            // First line, may contain position token.
            // Line may be modified.
            ass_pos_id_ = ExtractPosIdIfExists(line);
        }
        AppendLine(line);
    }
}

SubRipItem::SubRipItem(const SubRipItem &other) {
    start_ = other.start_;
    duration_ = other.duration_;
    ass_pos_id_ = other.ass_pos_id_;
    payload_ << other.payload_.str();
    num_lines_ = other.num_lines_;
}

SubRipItem &SubRipItem::operator=(const SubRipItem &other) {
    start_ = other.start_;
    duration_ = other.duration_;
    ass_pos_id_ = other.ass_pos_id_;
    // Move new one
    payload_ = std::ostringstream{};
    payload_ << other.payload_.str();
    num_lines_ = other.num_lines_;
    return *this;
}

void SubRipItem::ToStream(std::size_t sequence_number,
                          std::ostream &output) const {
    // Refer to https://docs.fileformat.com/video/srt/
    // Line 1: the subtitle sequence id.
    output << sequence_number << std::endl;

    // Line 2: The timestamp.
    output << ToSubRipDuration(start_);
    output << " --> ";
    output << ToSubRipDuration(start_ + duration_) << std::endl;

    // TODO: extended SRT format for specifying location of subtitle

    // Line 3 and onwards: Subtitle lines and styling.
    auto payload = payload_.str();
    if (!payload.empty()) {
        if (ass_pos_id_) {
            output << "{\\an" << *ass_pos_id_ << "}";
        }
        output << payload;
    }
}

bool SubRipItem::operator<(const SubRipItem &other) const {
    if (start_ != other.start_) {
        return start_ < other.start_;
    }
    return duration_ < other.duration_;
}

SubRipItem *SubRipItem::AppendLine(const std::string &payload) {
    payload_ << payload;
    if (!payload.empty() && payload.back() != '\n') {
        payload_ << std::endl;
    }
    ++num_lines_;
    return this;
}

SubRipItem *SubRipItem::ClearPayload() {
    payload_ = std::ostringstream{};
    num_lines_ = 0;
    return this;
}

}  // namespace srt
}  // namespace subtitler
