#ifndef SUBTITLER_SRT_SUBRIP_ITEM_H
#define SUBTITLER_SRT_SUBRIP_ITEM_H

#include <chrono>
#include <optional>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace subtitler {
namespace srt {

// Internal representation of a SRT subtitle item.
class SubRipItem {
public:
    SubRipItem() = default;

    void ToStream(std::size_t position, std::ostream &output);

    // Sort by start and then by duration.
    bool operator<(const SubRipItem &other) const;

    SubRipItem* start(std::chrono::milliseconds start) { start_ = start; return this; }

    SubRipItem* duration(std::chrono::milliseconds duration) { duration_ = duration; return this; }

    SubRipItem* append_line(const std::string &payload) { payload_ << payload << std::endl; return this; }

    SubRipItem* clear_payload() { payload_ = std::ostringstream{}; return this; }

    // Throws out_of_range if invalid position provided.
    SubRipItem* position(const std::string &position_id) { ass_pos_id_ = pos_to_id.at(position_id); return this; }

    static const inline std::string substation_alpha_positions{
        "top-left    top-center    top-right\n"
        "middle-left middle-center middle-right\n"
        "bottom-left bottom-center bottom-right"
    };
    static const inline std::unordered_map<std::string, int> pos_to_id{
        {"top-left", 7},
        {"top-center", 8},
        {"top-right", 9},
        {"middle-left", 4},
        {"middle-center", 5},
        {"middle-right", 6},
        {"bottom-left", 1},
        {"bottom-center", 2},
        {"bottom-right", 3}
    };

private:
    std::chrono::milliseconds start_;
    std::chrono::milliseconds duration_;
    std::optional<int> ass_pos_id_;
    std::ostringstream payload_;
};

} // namespace srt
} // namespace subtitler

#endif