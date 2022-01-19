#include "subtitler/cli/io/input.h"
#include "subtitler/util/unicode.h"

namespace subtitler::cli::io {

WideInputGetter::WideInputGetter(std::wistream &stream)
    : InputGetter{}, stream_{stream} {}

bool WideInputGetter::getline(std::string &line) {
    std::wstring temp;
    if (!std::getline(stream_, temp)) {
        return false;
    }
    line = ConvertFromWString(temp);
    return true;
}

}  // namespace subtitler::cli::io
