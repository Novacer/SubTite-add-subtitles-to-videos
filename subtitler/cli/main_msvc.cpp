#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <filesystem>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <io.h>
#include <fcntl.h>

#pragma comment(lib, "comdlg32.lib")
#include <windows.h>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/play_video/ffplay.h"
#include "subtitler/cli/commands.h"
#include "subtitler/cli/io/input.h"
#include "subtitler/util/unicode.h"

DEFINE_string(ffplay_path, "ffplay", "Required. Path to ffplay binary.");
DEFINE_string(ffmpeg_path, "ffmpeg", "Required. Path to ffmpeg binary.");
DEFINE_string(ffprobe_path, "ffprobe", "Required. Path to ffprobe binary.");

namespace {

// Checks that the value of the flag is not empty string.
bool ValidateFlagNonEmpty(const char *flagname, const std::string &value) {
    return !value.empty();
}

// Checks that binaries FFmpeg, FFplay, FFprobe all exist.
void ValidateFFBinaries() {
    subtitler::subprocess::SubprocessExecutor executor;
    executor.CaptureOutput(true);

    for (const auto &binary: {FLAGS_ffplay_path, FLAGS_ffmpeg_path, FLAGS_ffprobe_path}) {
        executor.SetCommand(binary + " -version");
        executor.Start();
        auto output = executor.WaitUntilFinished(5000);
        if (output.subproc_stdout.empty() || !output.subproc_stderr.empty()) {
            throw std::runtime_error("Error trying to detect binary at " + binary);
        }
    }
}

// Make a best effort to fix otherwise valid input paths.
// For example, in some cases we need quotes when passing input paths like "path/to some/folder"
// as a command line argument. While in some other cases (opening file directly) we don't want to use quotes.
void FixInputPath(std::string &path, bool should_have_quotes) {
    if (path.empty() || path.length() < 2) {
        return;
    }
    bool already_in_quotes = path.front() == '"' && path.back() == '"';
    if (should_have_quotes && !already_in_quotes) {
        // Any input paths with a space in them needs to be wrapped in quotes.
        bool contains_space = path.find(' ') != std::string::npos;
        if (contains_space) {
            std::ostringstream stream;
            stream << '"' << path << '"';
            path = stream.str();
        }
    } else if (!should_have_quotes && already_in_quotes) {
        // Have to strip the quotes.
        path = path.substr(1, path.length() - 2);
    }
}

// Opens a window file dialog to get the file paths.
// This is the preferred method, since typing the path through terminal
// may mess up the character encodings.
void GetFileFromDialog(WCHAR *path_buffer, const std::wstring &dialog_title, bool save) {
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = NULL;
    ofn.lpstrFile = path_buffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = dialog_title.c_str();
    ofn.Flags = OFN_DONTADDTORECENT;

    if (!save) {
        ofn.Flags |= OFN_FILEMUSTEXIST;
        if (!GetOpenFileNameW( &ofn )) {
            throw std::runtime_error("Unable to open file");
        }
    } else {
        if (!GetSaveFileNameW( &ofn )) {
            throw std::runtime_error("Unable to save file");
        }
    }
}

} // namespace

DEFINE_validator(ffplay_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffmpeg_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffprobe_path, &ValidateFlagNonEmpty);

int main(int argc, char **argv) {
    using namespace subtitler;
    namespace fs = std::filesystem;

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

    // Needed to receive unicode input from terminal.
    // TODO: clients need chcp 65001 to see unicode output in terminal.
    _setmode(_fileno(stdin), _O_U16TEXT);
    
    try {
        ValidateFFBinaries();
    } catch (const std::runtime_error &e) {
        LOG(ERROR) << e.what();
        LOG(ERROR) << "Please verify you have ffmpeg, ffplay, and ffpath installed.";
        LOG(ERROR) << "You can either add these to your PATH variable and the CLI can detect them automatically.";
        LOG(ERROR) << "Or you may provide the paths using --ffmpeg_path, --ffprobe_path, --ffplay_path flags";
        return 1;
    }

    LOG(INFO) << "Successfully detected all binaries!";

    WCHAR video_path_buffer[MAX_PATH] = {0};
    WCHAR output_subtitle_path_buffer[MAX_PATH] = {0};
    try {
        std::cout << "Please select the video file you want to subtitle." << std::endl;
        GetFileFromDialog(video_path_buffer, L"Choose video path", false);
    } catch(const std::runtime_error &e) {
        LOG(ERROR) << "Unable to open video file. Please try again.";
        return 1;
    }
    try {
        std::cout << "Please select the output subtitle file." << std::endl;
        GetFileFromDialog(output_subtitle_path_buffer, L"Choose output path", true);
    } catch(const std::runtime_error &e) {
        LOG(ERROR) << "Unable to open output subtitle file. Please try again.";
        return 1;
    }
    
    auto video_path = ConvertFromWStr(video_path_buffer);
    auto output_subtitle_path = ConvertFromWStr(output_subtitle_path_buffer);

    // Make sure input file path is wrapped/unwrapped with quotes as needed.
    // Paths passed to command args should have quotes
    // Paths opened directly should not have quotes.
    // TODO: determine whether the FF binaries should have quotes or not.
    FixInputPath(video_path, /* should_have_quotes= */ true);
    FixInputPath(output_subtitle_path, /* should_have_quotes= */ false);

    // Sanity test writing to output path beforehand so we know it works.
    {
        auto path_wrapper = fs::path(fs::u8path(output_subtitle_path));
        // Write empty string in append mode.
        std::ofstream ofs{path_wrapper, std::ios_base::app};
        if (!ofs) {
            LOG(ERROR) << "Unable to open file: " << output_subtitle_path;
            return 1;
        }
        ofs << "";
    }

    auto executor = std::make_unique<subprocess::SubprocessExecutor>();
    auto ffplay = std::make_unique<play_video::FFPlay>(FLAGS_ffplay_path, std::move(executor));
    cli::Commands::Paths paths{video_path, output_subtitle_path};
    cli::Commands commands{
        paths,
        std::move(ffplay),
        std::make_unique<cli::io::WideInputGetter>(std::wcin),
        std::cout};

    try {
        commands.MainLoop();
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what();
        return 1;
    }
    
    return 0;
}
