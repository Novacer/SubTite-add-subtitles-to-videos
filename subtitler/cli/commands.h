#ifndef SUBTITLER_CLI_COMMANDS_H
#define SUBTITLER_CLI_COMMANDS_H

#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "subtitler/srt/subrip_file.h"

// Forward declaration
namespace subtitler::video::player {

class FFPlay;

}  // namespace subtitler::video::player

namespace subtitler::video::metadata {

struct Metadata;

}  // namespace subtitler::video::metadata

namespace subtitler::cli::io {

class InputGetter;

}  // namespace subtitler::cli::io

namespace subtitler {

class TempFile;

}  // namespace subtitler

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

    Commands(const Paths &paths, std::unique_ptr<video::player::FFPlay> ffplay,
             std::unique_ptr<io::InputGetter> input_getter,
             std::ostream &output,
             std::unique_ptr<video::metadata::Metadata> metadata);

    ~Commands();

    Commands(const Commands &) = delete;
    Commands &operator=(const Commands &) = delete;

    // Initialize and start reading commands from input stream.
    // Any outputs are written to output stream.
    // Use command help to get list of all commands.
    void MainLoop();

  private:
    Paths paths_;
    std::unique_ptr<video::player::FFPlay> ffplay_;
    std::unique_ptr<io::InputGetter> input_getter_;
    std::ostream &output_;
    std::unique_ptr<video::metadata::Metadata> metadata_;
    std::chrono::milliseconds start_;
    std::chrono::milliseconds duration_;
    srt::SubRipFile srt_file_;
    bool srt_file_has_changed_;
    std::unique_ptr<TempFile> temp_file_;

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

}  // namespace cli
}  // namespace subtitler

#endif
