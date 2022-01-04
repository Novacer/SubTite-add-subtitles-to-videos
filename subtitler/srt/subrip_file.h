#ifndef SUBTITLER_SRT_SUBRIP_FILE_H
#define SUBTITLER_SRT_SUBRIP_FILE_H

#include <chrono>
#include <optional>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>

namespace subtitler {
namespace srt {

// Forward declaration
class SubRipItem;

// Internal representation of an SRT subtitle file.
class SubRipFile {
public:
    SubRipFile() = default;

    void ToStream(std::ostream &output) const;

    std::size_t NumItems() const;

    // Just do linear search for all items with this.end < other.start. If other starts after I end,
    // then no way we can intersect. Note that existing intervals may overlap, so direct binary search won't work.
    // Other solution is to use interval tree, but until we actually need O(log n) time let's Keep It Simple.
    std::unordered_map<std::size_t, const SubRipItem *> GetCollisions(
        std::chrono::milliseconds start,
        std::chrono::milliseconds duration);

    void AddItem(const SubRipItem &item);

private:
    // SRT items should be ordered by start time to allow for overlapping subtitles.
    std::vector<SubRipItem> items_;
};

} // namespace srt
} // namespace subtitler

#endif
