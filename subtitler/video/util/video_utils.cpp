#include "subtitler/video/util/video_utils.h"

#include <sstream>

namespace subtitler {
namespace video {
namespace util {

std::string FixPathForFilters(const std::string &path) {
    std::ostringstream output;
    for (const auto &c : path) {
        if (c == '\\') {
            output << '/';
        } else if (c == ':') {
            output << "\\:";
        } else {
            output << c;
        }
    }
    return output.str();
}

}  // namespace util
}  // namespace video
}  // namespace subtitler
