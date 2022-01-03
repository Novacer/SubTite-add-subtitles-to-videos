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

/**
 * Used to parse and run commands in the CLI.
 */
class Commands {
public:
    struct Paths {
        std::string video_path;
    };

    // Input and output streams must have greater lifetime than this object.
    Commands(const Paths &paths,
        std::unique_ptr<play_video::FFPlay> ffplay,
        std::istream &input,
        std::ostream &output);
    
    ~Commands();
    
    Commands(const Commands &) = delete;
    Commands& operator=(const Commands &) = delete;

    // Initialize and start reading commands from input stream.
    // Any outputs are written to output stream.
    // Use command help to get list of all commands.
    void MainLoop();

private:
    Paths paths_;
    std::unique_ptr<play_video::FFPlay> ffplay_;
    std::istream &input_;
    std::ostream &output_;
    std::chrono::milliseconds start_;
    std::chrono::milliseconds duration_;

    void Help();

    void Play(const std::vector<std::string> &tokens);
};

} // namespace cli
} // namespace subtitler

#endif
