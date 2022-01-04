#include "subtitler/srt/subrip_item.h"

#include "subtitler/util/duration_format.h"
#include <string>

namespace subtitler {
namespace srt {

SubRipItem::SubRipItem(const SubRipItem &other) {
    start_ = other.start_;
    duration_ = other.duration_;
    ass_pos_id_ = other.ass_pos_id_;
    payload_ << other.payload_.str();
}

SubRipItem& SubRipItem::operator=(const SubRipItem &other) {
    start_ = other.start_;
    duration_ = other.duration_;
    ass_pos_id_ = other.ass_pos_id_;
    // Move new one
    payload_ = std::ostringstream{};
    payload_ << other.payload_.str();
    return *this;
}

void SubRipItem::ToStream(std::size_t sequence_number, std::ostream &output) const {
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
            output << "{an" << *ass_pos_id_ << "} ";
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

} // namespace srt
} // namespace subtitler
