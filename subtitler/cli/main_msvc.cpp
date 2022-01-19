#include <iostream>
#include <stdexcept>
#include <memory>
#include <fstream>
#include <filesystem>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <io.h>
#include <fcntl.h>

#pragma comment(lib, "ole32.lib")
#include <windows.h>
#include <shobjidl.h> 

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/video/player/ffplay.h"
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

// Windows specific function to open a dialog for selecting file paths.
// If save is true, the file will be treated as an output file.
// If save is false, the file to be opened must exist.
std::wstring OpenFileDialog(bool save) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        throw std::runtime_error("Could not initialize COM library");
    }
    IFileDialog *pFileDialog;

    // Create the FileOpenDialog object.
    if (save) {
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, 
            IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileDialog));
    } else {
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileDialog));
    }
    
    if (FAILED(hr)) {
        CoUninitialize();
        throw std::runtime_error("Could not create FileDialog instance");
    }
    DWORD dwFlags = 0;
    hr = pFileDialog->GetOptions(&dwFlags);
    if (FAILED(hr)) {
        pFileDialog->Release();
        CoUninitialize();
        throw std::runtime_error("Could not retrieve internal dialog options");
    }
    hr = pFileDialog->SetOptions(dwFlags | FOS_DONTADDTORECENT);
    if (FAILED(hr)) {
        pFileDialog->Release();
        CoUninitialize();
        throw std::runtime_error("Could not set internal dialog options");
    }
    hr = pFileDialog->Show(NULL);
    if (FAILED(hr)) {
        pFileDialog->Release();
        CoUninitialize();
        throw std::runtime_error("Could not select file");
    }
    IShellItem *pItem;
    hr = pFileDialog->GetResult(&pItem);
    if (FAILED(hr)) {
        pFileDialog->Release();
        CoUninitialize();
        throw std::runtime_error("Could not get result from file selection");
    }
    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
    if (FAILED(hr)) {
        pItem->Release();
        pFileDialog->Release();
        CoUninitialize();
        throw std::runtime_error("Could not get display name of selected file");
    }
    std::wstring result{pszFilePath};

    // Cleanup
    CoTaskMemFree(pszFilePath);
    pItem->Release();
    pFileDialog->Release();
    CoUninitialize();

    if (result.empty()) {
        throw std::runtime_error("Unexpected empty path selected!");
    }
    return result;
}

} // namespace

DEFINE_validator(ffplay_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffmpeg_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffprobe_path, &ValidateFlagNonEmpty);

int main(int argc, char **argv) {
    using namespace subtitler;
    namespace fs = std::filesystem;

    // Temporary workaround until Bill Gates fixes this deadlock with ASAN and OpenFileDialog
    // More info: https://stackoverflow.com/a/69718929/17786559
    SetProcessAffinityMask(GetCurrentProcess(), 1);

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

    std::string video_path;
    std::string output_subtitle_path;
    try {
        std::cout << "Please select the video file you want to subtitle." << std::endl;
        video_path = ConvertFromWString(OpenFileDialog(false));
    } catch(const std::runtime_error &e) {
        LOG(INFO) << e.what();
        LOG(ERROR) << "Unable to open video file. Please try again.";
        return 1;
    }
    try {
        std::cout << "Please select the output subtitle file." << std::endl;
        output_subtitle_path = ConvertFromWString(OpenFileDialog(true));
    } catch(const std::runtime_error &e) {
        LOG(INFO) << e.what();
        LOG(ERROR) << "Unable to open output subtitle file. Please try again.";
        return 1;
    }

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

    cli::Commands::Paths paths{video_path, output_subtitle_path};
    auto executor = std::make_unique<subprocess::SubprocessExecutor>();
    auto ffplay = std::make_unique<video::player::FFPlay>(FLAGS_ffplay_path, std::move(executor));
    auto wide_input_getter = std::make_unique<cli::io::WideInputGetter>(std::wcin);
    
    cli::Commands commands{
        paths,
        std::move(ffplay),
        std::move(wide_input_getter),
        std::cout};

    try {
        commands.MainLoop();
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what();
        return 1;
    }
    
    return 0;
}
