#ifndef SUBTITLER_SRT_SUBRIP_ITEM_H
#define SUBTITLER_SRT_SUBRIP_ITEM_H

#include <chrono>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace subtitler {
namespace srt {

// Internal representation of a SRT subtitle item.
class SubRipItem {
  public:
    SubRipItem() = default;
    explicit SubRipItem(const std::string& payload);
    SubRipItem(const SubRipItem& other);
    SubRipItem& operator=(const SubRipItem& other);
    // Sort by start and then by duration.
    bool operator<(const SubRipItem& other) const;

    // Outputs the subrip item. Format:
    // position
    // start_ --> start + duration
    // styling and subtitles...
    void ToStream(std::size_t sequence_number, std::ostream& output) const;

    SubRipItem* start(std::chrono::milliseconds start) {
        start_ = start;
        return this;
    }

    SubRipItem* duration(std::chrono::milliseconds duration) {
        duration_ = duration;
        return this;
    }

    SubRipItem* append_line(const std::string& payload);

    SubRipItem* clear_payload();

    // Throws out_of_range if invalid position provided.
    SubRipItem* position(const std::string& position_id) {
        ass_pos_id_ = pos_to_id.at(position_id);
        return this;
    }

    int num_lines() const { return num_lines_; }

    // Used to describe the possible positions.
    static const inline std::string substation_alpha_positions{
        "top-left    top-center    top-right\n"
        "middle-left middle-center middle-right\n"
        "bottom-left bottom-center bottom-right"};

    // Maps position to its substation alpha id.
    static const inline std::unordered_map<std::string, int> pos_to_id{
        {"top-left", 7},      {"tl", 7}, {"top-center", 8},    {"tc", 8},
        {"top-right", 9},     {"tr", 9}, {"middle-left", 4},   {"ml", 4},
        {"middle-center", 5}, {"mc", 5}, {"middle-right", 6},  {"mr", 6},
        {"bottom-left", 1},   {"bl", 1}, {"bottom-center", 2}, {"bc", 2},
        {"bottom-right", 3},  {"br", 3}};

  private:
    std::chrono::milliseconds start_;
    std::chrono::milliseconds duration_;
    std::optional<int> ass_pos_id_;
    std::ostringstream payload_;
    int num_lines_ = 0;

    friend class SubRipFile;
};

}  // namespace srt
}  // namespace subtitler

#endif
