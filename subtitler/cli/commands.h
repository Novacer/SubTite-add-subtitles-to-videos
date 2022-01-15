#ifndef SUBTITLER_CLI_COMMANDS_H
#define SUBTITLER_CLI_COMMANDS_H

#include <sstream>
#include <string>
#include <chrono>
#include <vector>
#include <memory>
#include "subtitler/srt/subrip_file.h"

// Forward declaration
namespace subtitler::play_video {

class FFPlay;

} // namespace subtitler::play_video

namespace subtitler::cli::io {

class InputGetter;

} // namespace subtitler::cli::io

namespace subtitler {

class TempFile;

} // namespace subtitler::play_video

namespace subtitler {
namespace cli {

/**
 * Used to parse and run commands in the CLI.
 */
class Commands {
public:
    struct Paths {
        std::string video_path;
        std::string output_subtitle_path;
    };

    Commands(const Paths &paths,
        std::unique_ptr<play_video::FFPlay> ffplay,
        std::unique_ptr<io::InputGetter> input_getter,
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
    std::ostream &output_;
    std::chrono::milliseconds start_;
    std::chrono::milliseconds duration_;
    srt::SubRipFile srt_file_;
    bool srt_file_has_changed_;
    std::unique_ptr<TempFile> temp_file_;
    std::unique_ptr<io::InputGetter> input_getter_;

    void Help();

    void Play(const std::vector<std::string> &tokens);

    void PrintSubs();
    void AddSub(const std::vector<std::string> &tokens);
    void DeleteSub(const std::vector<std::string> &tokens);
    void EditSub(const std::vector<std::string> &tokens);
    void Save();

    void Quit();

    void _GeneratePreviewSubs();
};

} // namespace cli
} // namespace subtitler

#endif
