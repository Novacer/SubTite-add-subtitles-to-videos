// Experimental binary using ffmpeg to trim videos.
// TODO: clean this up.

#include <fcntl.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <io.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

#pragma comment(lib, "ole32.lib")
#include <shobjidl.h>
#include <windows.h>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/util/duration_format.h"
#include "subtitler/util/temp_file.h"
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

    for (const auto &binary :
         {FLAGS_ffplay_path, FLAGS_ffmpeg_path, FLAGS_ffprobe_path}) {
        executor.SetCommand(binary + " -version");
        executor.Start();
        auto output = executor.WaitUntilFinished(5000);
        if (output.subproc_stdout.empty() || !output.subproc_stderr.empty()) {
            throw std::runtime_error("Error trying to detect binary at " +
                                     binary);
        }
    }
}

// Make a best effort to fix otherwise valid input paths.
// For example, in some cases we need quotes when passing input paths like
// "path/to some/folder" as a command line argument. While in some other cases
// (opening file directly) we don't want to use quotes.
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
    HRESULT hr =
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        throw std::runtime_error("Could not initialize COM library");
    }
    IFileDialog *pFileDialog;

    // Create the FileOpenDialog object.
    if (save) {
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                              IID_IFileSaveDialog,
                              reinterpret_cast<void **>(&pFileDialog));
    } else {
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                              IID_IFileOpenDialog,
                              reinterpret_cast<void **>(&pFileDialog));
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

struct Interval {
    std::optional<std::chrono::milliseconds> start;
    std::optional<std::chrono::milliseconds> end;
};

}  // namespace

DEFINE_validator(ffplay_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffmpeg_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffprobe_path, &ValidateFlagNonEmpty);

int main(int argc, char **argv) {
    using namespace subtitler;
    namespace fs = std::filesystem;

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

#ifdef _DEBUG
    LOG(INFO) << "debug mode";
    // Temporary workaround until Bill Gates fixes this deadlock with ASAN and
    // OpenFileDialog More info: https://stackoverflow.com/a/69718929/17786559
    SetProcessAffinityMask(GetCurrentProcess(), 1);
#endif

    // Needed to receive unicode input from terminal.
    // TODO: clients need chcp 65001 to see unicode output in terminal.
    _setmode(_fileno(stdin), _O_U16TEXT);

    try {
        ValidateFFBinaries();
    } catch (const std::runtime_error &e) {
        LOG(ERROR) << e.what();
        LOG(ERROR)
            << "Please verify you have ffmpeg, ffplay, and ffpath installed.";
        LOG(ERROR) << "You can either add these to your PATH variable and the "
                      "CLI can detect them automatically.";
        LOG(ERROR) << "Or you may provide the paths using --ffmpeg_path, "
                      "--ffprobe_path, --ffplay_path flags";
        return 1;
    }

    LOG(INFO) << "Successfully detected all binaries!";

    std::string video_path;
    std::string timestamp_path;
    std::string output_path;
    try {
        std::cout << "Please select the video file you trim." << std::endl;
        auto temp = OpenFileDialog(/* save= */ false);
        LOG(INFO) << temp.size();
        video_path = ConvertFromWString(temp);
    } catch (const std::runtime_error &e) {
        LOG(INFO) << e.what();
        LOG(ERROR) << "Unable to open video file. Please try again.";
        return 1;
    }
    try {
        std::cout << "Please select the time stamp file." << std::endl;
        auto temp = OpenFileDialog(/* save= */ false);
        LOG(INFO) << temp.size();
        timestamp_path = ConvertFromWString(temp);
    } catch (const std::runtime_error &e) {
        LOG(INFO) << e.what();
        LOG(ERROR) << "Unable to open timestamp file. Please try again.";
        return 1;
    }
    try {
        std::cout << "Please select output file name." << std::endl;
        auto temp = OpenFileDialog(/* save= */ true);
        LOG(INFO) << temp.size();
        output_path = ConvertFromWString(temp);
    } catch (const std::runtime_error &e) {
        LOG(INFO) << e.what();
        LOG(ERROR) << "Unable to open output file. Please try again.";
        return 1;
    }

    LOG(INFO) << video_path;
    LOG(INFO) << timestamp_path;
    LOG(INFO) << output_path;

    // IMPORTANT - This must be generated BEFORE the path is fixed with ""
    auto output_path_wrapper = fs::path(fs::u8path(output_path));

    // Make sure input file path is wrapped/unwrapped with quotes as needed.
    // Paths passed to command args should have quotes
    // Paths opened directly should not have quotes.
    // TODO: determine whether the FF binaries should have quotes or not.
    FixInputPath(video_path, /* should_have_quotes= */ true);
    FixInputPath(timestamp_path, /* should_have_quotes= */ false);
    FixInputPath(output_path, /* should_have_quotes= */ true);

    auto path_wrapper = fs::path(fs::u8path(timestamp_path));

    std::ifstream stream{path_wrapper};
    std::string read_buffer;
    std::vector<Interval> intervals;

    LOG(INFO) << "Reading from file";

    while (std::getline(stream, read_buffer)) {
        if (read_buffer.empty()) {
            continue;
        }
        std::istringstream line{read_buffer};
        std::string token;
        std::optional<std::chrono::milliseconds> start;
        std::optional<std::chrono::milliseconds> end;

        if (!(line >> token)) {
            LOG(ERROR) << "Parse error: unable to get start time.";
            return 1;
        }
        start = subtitler::ParseDuration(token);
        if (!start) {
            LOG(ERROR) << "wrong start time format";
        }
        if (!(line >> token) || token != "-") {
            LOG(ERROR) << "Parse error: expected -";
            return 1;
        }
        if (line >> token) {
            end = subtitler::ParseDuration(token);
        }
        intervals.push_back(Interval{start, end});
        if (!end) {
            break;
        }
    }
    stream.close();

    LOG(INFO) << "Begin ffmpeg extract intervals...";
    std::vector<std::unique_ptr<subtitler::TempFile>> temp_files;
    std::unique_ptr<subtitler::TempFile> file_containing_file_names;
    try {
        LOG(INFO) << "before alloc";
        file_containing_file_names = std::make_unique<subtitler::TempFile>(
            "", output_path_wrapper.parent_path(), ".txt");
        LOG(INFO) << "after alloc";
        if (!file_containing_file_names) {
            throw std::runtime_error("I don't know why it didn't alloc?");
        }
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what();
        return 1;
    }
    std::ofstream file_names{
        fs::path(fs::u8path(file_containing_file_names->FileName()))};
    LOG(INFO) << "Created the first temp file...";

    subtitler::subprocess::SubprocessExecutor executor{};
    executor.CaptureOutput(true);

    for (const auto &interval : intervals) {
        auto start_str = subtitler::FormatDuration(*interval.start);
        std::string end_str = "";
        if (interval.end) {
            end_str = subtitler::FormatDuration(*interval.end);
        }

        LOG(INFO) << "Start interval";

        std::ostringstream builder;
        builder << "ffmpeg -loglevel error -y -ss " << start_str;
        if (!end_str.empty()) {
            builder << " -to " << end_str;
        }
        builder << " -i " << video_path;
        builder << " -f matroska -c copy ";

        try {
            temp_files.emplace_back(std::make_unique<subtitler::TempFile>(
                "", output_path_wrapper.parent_path(), ".mkv"));
        } catch (const std::exception &e) {
            LOG(ERROR) << e.what();
            return 1;
        }

        auto temp_file_name = (*temp_files.rbegin())->FileName();
        builder << '"' << temp_file_name << '"';

        LOG(INFO) << "Running: " << builder.str();
        executor.SetCommand(builder.str());

        try {
            executor.Start();
            auto output = executor.WaitUntilFinished();
            if (!output.subproc_stderr.empty()) {
                LOG(ERROR) << "Encountered error running: " << builder.str();
                LOG(ERROR) << output.subproc_stderr;
                return 1;
            }
        } catch (const std::exception &e) {
            LOG(ERROR) << e.what();
            return 1;
        }

        LOG(INFO) << "Subproc complete, now writing";
        // Use the original filename without modifications.
        file_names << "file '" << temp_file_name << "'" << std::endl;
    }

    file_names.close();

    LOG(INFO) << "Split done, now starting concat";
    // Concat the filenames
    std::ostringstream builder;
    builder << "ffmpeg -safe 0 -loglevel error -y -f concat -i ";
    builder << '"' << file_containing_file_names->FileName() << '"';
    builder << " -c copy " << output_path;

    LOG(INFO) << "Running " << builder.str();

    executor.SetCommand(builder.str());

    try {
        executor.Start();
        auto output = executor.WaitUntilFinished();
        if (!output.subproc_stderr.empty()) {
            LOG(ERROR) << "Encountered error running: " << builder.str();
            LOG(ERROR) << output.subproc_stderr;
            return 1;
        }
    } catch (const std::exception &e) {
        LOG(ERROR) << e.what();
        return 1;
    }

    return 0;
}
