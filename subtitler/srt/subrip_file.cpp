#include "subtitler/srt/subrip_file.h"

#include <algorithm>
#include <stdexcept>

#include "subtitler/srt/subrip_item.h"

namespace subtitler {
namespace srt {

void SubRipFile::ToStream(std::ostream &output) const {
    for (std::size_t i = 0; i < items_.size(); ++i) {
        items_[i].ToStream(i + 1, output);
        output << std::endl;
    }
}

void SubRipFile::ToStream(std::ostream &output, std::chrono::milliseconds start,
                          std::chrono::milliseconds duration) const {
    std::size_t sequence_number = 1;

    auto print_item = [&](std::size_t ignored, const SubRipItem &item) {
        // To produce a valid SRT file, the first subtitle must begin with
        // sequence one. Hence we provide our own sequential counter while
        // ignoring the index.
        item.ToStream(sequence_number, output);
        output << std::endl;
        ++sequence_number;
    };
    this->ForEachOverlappingItem(start, duration, print_item);
}

std::size_t SubRipFile::NumItems() const { return items_.size(); }

std::unordered_map<std::size_t, const SubRipItem *> SubRipFile::GetCollisions(
    std::chrono::milliseconds start, std::chrono::milliseconds duration) const {
    std::unordered_map<std::size_t, const SubRipItem *> intersections;

    auto add_to_intersections = [&](std::size_t index, const SubRipItem &item) {
        // Recall that sequence number is 1's based, while vector index is 0's
        // based.
        intersections[index + 1] = &item;
    };
    this->ForEachOverlappingItem(start, duration, add_to_intersections);

    return intersections;
}

void SubRipFile::AddItem(const SubRipItem &item) {
    // Inserts in sorted order
    auto insert_here = std::lower_bound(items_.begin(), items_.end(), item);
    items_.insert(insert_here, item);
}

SubRipItem SubRipFile::RemoveItem(std::size_t sequence_number) {
    if (sequence_number <= 0 || sequence_number > NumItems()) {
        throw std::out_of_range("invalid index passed to RemoveItem()");
    }
    auto vector_position = sequence_number - 1;
    auto backup = items_.at(vector_position);
    items_.erase(items_.begin() + vector_position);
    return backup;
}

void SubRipFile::EditItemPosition(std::size_t sequence_number,
                                  const std::string &position) {
    if (sequence_number <= 0 || sequence_number > NumItems()) {
        throw std::out_of_range("invalid index passed to EditItemPosition()");
    }
    auto vector_position = sequence_number - 1;
    auto &item = items_.at(vector_position);
    item.position(position);
}

void SubRipFile::ForEachOverlappingItem(
    std::chrono::milliseconds start, std::chrono::milliseconds duration,
    std::function<void(std::size_t, const SubRipItem &)> on_find) const {
    // Run linear search to collect intersections. Since we allow overlaps in
    // existing intervals, there is not a trivial binary search that we can do.
    // A better way would be to use interval tree, but until we see an actual
    // need to use one let's Keep it Simple!
    auto end = start + duration;
    for (std::size_t i = 0; i < items_.size(); ++i) {
        auto &item = items_.at(i);
        // Assume items are sorted by start time. If the next interval starts
        // after I end then none of the remaining items can overlap with me.
        if (item.start_ > end) {
            break;
        }
        auto item_end = item.start_ + item.duration_;
        // Reference https://stackoverflow.com/questions/3269434
        auto does_overlap = start <= item_end && item.start_ <= end;
        if (does_overlap) {
            on_find(i, item);
        }
    }
}

}  // namespace srt
}  // namespace subtitler
