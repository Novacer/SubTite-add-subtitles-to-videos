#ifndef SUBTITLER_CLI_COMMANDS_H
#define SUBTITLER_CLI_COMMANDS_H

#include <sstream>
#include <string>
#include <chrono>
#include <vector>
#include <memory>

// Forward declaration
namespace subtitler::play_video {

class FFPlay;

} // subtitler::play_video

namespace subtitler {
namespace cli {

class Commands {
public:
    struct Paths {
        std::string video_path;
    };

    Commands(const Paths &paths,
        std::unique_ptr<play_video::FFPlay> ffplay,
        std::istream &input,
        std::ostream &output);
    
    ~Commands();

    void MainLoop();

    void Help();

    void Play(const std::vector<std::string> &tokens);

private:
    Paths paths_;
    std::unique_ptr<play_video::FFPlay> ffplay_;
    std::istream &input_;
    std::ostream &output_;
    std::chrono::milliseconds start_;
    std::chrono::milliseconds duration_;
};

} // namespace cli
} // namespace subtitler

#endif
