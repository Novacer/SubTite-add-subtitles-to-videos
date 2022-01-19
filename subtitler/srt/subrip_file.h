#ifndef SUBTITLER_SRT_SUBRIP_FILE_H
#define SUBTITLER_SRT_SUBRIP_FILE_H

#include <chrono>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace subtitler {
namespace srt {

// Forward declaration
class SubRipItem;

// Internal representation of an SRT subtitle file.
class SubRipFile {
  public:
    SubRipFile() = default;

    // Prints the entire SubRipFile to the stream.
    void ToStream(std::ostream &output) const;

    // Prints a valid SRT file with only the SubRipItems
    // that have non-empty intersections with [start, start + end].
    void ToStream(std::ostream &output, std::chrono::milliseconds start,
                  std::chrono::milliseconds duration) const;

    // Return the number of SubRipItems.
    std::size_t NumItems() const;

    // Returns the SubRipItems which have start and ends intersecting with the
    // given interval. SubRipItems are keyed by sequence number. Worst case this
    // is a linear search.
    std::unordered_map<std::size_t, const SubRipItem *> GetCollisions(
        std::chrono::milliseconds start,
        std::chrono::milliseconds duration) const;

    // Add a SubRipItem. Maintains sorted order by start time.
    // Overlapping intervals are allowed.
    void AddItem(const SubRipItem &item);

    // Remove the SubRipItem with sequence number and returns the removed one.
    // Throws std::out_of_range if invalid sequence is provided.
    // Note that indices start at 1, as per SRT file format spec.
    SubRipItem RemoveItem(std::size_t sequence_number);

    // Edits the positioning of an existing SubRipItem.
    // Reference SubRipItem::pos_to_id for valid positions.
    // Throws std::out_of_range if invalid sequence or position is provided.
    void EditItemPosition(std::size_t sequence_number,
                          const std::string &position);

  private:
    // SRT items should be ordered by start time to allow for overlapping
    // subtitles.
    std::vector<SubRipItem> items_;

    // Find all intervals with non-empty intersection with [start, start +
    // duration] For each item found, call on_find(index_of_item, item).
    void ForEachOverlappingItem(
        std::chrono::milliseconds start, std::chrono::milliseconds duration,
        std::function<void(std::size_t, const SubRipItem &)> on_find) const;
};

}  // namespace srt
}  // namespace subtitler

#endif
