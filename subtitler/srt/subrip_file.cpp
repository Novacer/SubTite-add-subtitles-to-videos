#include "subtitler/srt/subrip_file.h"

#include <algorithm>
#include "subtitler/srt/subrip_item.h"

namespace subtitler {
namespace srt {

void SubRipFile::ToStream(std::ostream &output) const {
    for (std::size_t i = 0; i < items_.size(); ++i) {
        items_[i].ToStream(i + 1, output);
        output << std::endl;
    }
}

std::size_t SubRipFile::NumItems() const {
    return items_.size();
}

std::unordered_map<std::size_t, const SubRipItem *> SubRipFile::GetCollisions(
    std::chrono::milliseconds start,
    std::chrono::milliseconds duration) {
    // Run linear search to collect intersections.
    auto end = start + duration;
    std::unordered_map<std::size_t, const SubRipItem *> intersections;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        const auto &item = items_.at(i);
        // Assume items are sorted by start time. If the next interval starts after I end
        // then none of the remaining items can overlap with me.
        if (item.start_ > end) {
            break;
        }
        auto item_end = item.start_ + item.duration_;
        // Reference https://stackoverflow.com/questions/3269434
        auto does_overlap = start <= item_end && item.start_ <= end;
        if (does_overlap) {
            intersections[i] = &item;
        }
    }

    return intersections;
}

void SubRipFile::AddItem(const SubRipItem &item) {
    // Inserts in sorted order
    auto insert_here = std::lower_bound(items_.begin(), items_.end(), item);
    items_.insert(insert_here, item);
}

} // namespace srt
} // namespace subtitler
