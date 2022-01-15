#include "subtitler/cli/io/input.h"

namespace subtitler::cli::io{

WideInputGetter::WideInputGetter(std::wistream &stream): InputGetter{}, stream_{stream} {
    throw std::runtime_error("WideInputGetter is supported only on MSVC!");
}

bool WideInputGetter::getline(std::string &line) {
    throw std::runtime_error("WideInputGetter is supported only on MSVC!");
};

} // namespace subtitler::cli::io
