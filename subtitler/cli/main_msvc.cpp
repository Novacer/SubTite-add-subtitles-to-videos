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

#include "subtitler/cli/commands.h"
#include "subtitler/cli/io/input.h"
#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/util/unicode.h"
#include "subtitler/video/metadata/ffprobe.h"
#include "subtitler/video/player/ffplay.h"

DEFINE_string(ffplay_path, "ffplay", "Required. Path to ffplay binary.");
DEFINE_string(ffmpeg_path, "ffmpeg", "Required. Path to ffmpeg binary.");
DEFINE_string(ffprobe_path, "ffprobe", "Required. Path to ffprobe binary.");

namespace {

// Checks that the value of the flag is not empty string.
bool ValidateFlagNonEmpty(const char* flagname, const std::string& value) {
  return !value.empty();
}

// Checks that binaries FFmpeg, FFplay, FFprobe all exist.
void ValidateFFBinaries() {
  using subtitler::subprocess::SubprocessExecutor;
  std::vector<std::unique_ptr<SubprocessExecutor>> executors;
  std::vector<std::string> binaries = {FLAGS_ffplay_path, FLAGS_ffmpeg_path,
                                       FLAGS_ffprobe_path};
  executors.reserve(3);

  for (const auto& binary : binaries) {
    auto& executor = executors.emplace_back(
        std::make_unique<SubprocessExecutor>(binary + " -version",
                                             /* capture_output= */ true));
    executor->Start();
  }
  std::ostringstream errors;
  for (std::size_t i = 0; i < executors.size(); ++i) {
    auto& executor = executors.at(i);
    auto output = executor->WaitUntilFinished(2000);
    if (output.subproc_stdout.empty() || !output.subproc_stderr.empty()) {
      errors << " " << binaries.at(i);
    }
  }
  auto binary_error_list = errors.str();
  if (!binary_error_list.empty()) {
    throw std::runtime_error("Failed to detect:" + binary_error_list);
  }
}

// Make a best effort to fix otherwise valid input paths.
// For example, in some cases we need quotes when passing input paths like
// "path/to some/folder" as a command line argument. While in some other cases
// (opening file directly) we don't want to use quotes.
void FixInputPath(std::string& path, bool should_have_quotes) {
  if (path.empty() || path.length() < 2) {
    return;
  }
  bool already_in_quotes = (path.front() == '"' && path.back() == '"') ||
                           (path.front() == '\'' && path.back() == '\'');
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
  IFileDialog* pFileDialog;

  // Create the FileOpenDialog object.
  if (save) {
    hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                          IID_IFileSaveDialog,
                          reinterpret_cast<void**>(&pFileDialog));
  } else {
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                          IID_IFileOpenDialog,
                          reinterpret_cast<void**>(&pFileDialog));
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
  IShellItem* pItem;
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

}  // namespace

DEFINE_validator(ffplay_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffmpeg_path, &ValidateFlagNonEmpty);
DEFINE_validator(ffprobe_path, &ValidateFlagNonEmpty);

int main(int argc, char** argv) {
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

  // If any binary path has spaces, let's make sure they are not
  // interpreted wrongly by wrapping them up with quotes.
  FixInputPath(FLAGS_ffplay_path, /* should_have_quotes= */ true);
  FixInputPath(FLAGS_ffmpeg_path, /* should_have_quotes= */ true);
  FixInputPath(FLAGS_ffprobe_path, /* should_have_quotes= */ true);

  try {
    ValidateFFBinaries();
  } catch (const std::runtime_error& e) {
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
  std::string output_subtitle_path;
  try {
    std::cout << "Please select the video file you want to subtitle."
              << std::endl;
    video_path = ConvertFromWString(OpenFileDialog(/* save= */ false));
  } catch (const std::runtime_error& e) {
    LOG(INFO) << e.what();
    LOG(ERROR) << "Unable to open video file. Please try again.";
    return 1;
  }
  try {
    std::cout << "Please select the output subtitle file." << std::endl;
    output_subtitle_path = ConvertFromWString(OpenFileDialog(/* save= */ true));
  } catch (const std::runtime_error& e) {
    LOG(INFO) << e.what();
    LOG(ERROR) << "Unable to open output subtitle file. Please try again.";
    return 1;
  }

  // Sanity test writing to output path beforehand so we know it works.
  {
    auto path_wrapper = fs::u8path(output_subtitle_path);
    // Write empty string in append mode.
    std::ofstream ofs{path_wrapper, std::ios_base::app};
    if (!ofs) {
      LOG(ERROR) << "Unable to open file: " << output_subtitle_path;
      return 1;
    }
    ofs << "";
  }

  // Get video metadata.
  std::unique_ptr<video::metadata::Metadata> video_metadata;
  try {
    video::metadata::FFProbe ffprobe{
        FLAGS_ffprobe_path, std::make_unique<subprocess::SubprocessExecutor>()};
    video_metadata = ffprobe.GetVideoMetadata(video_path);
  } catch (const std::exception& e) {
    LOG(INFO) << e.what();
    LOG(ERROR) << "Unable to read video file: " << video_path;
    return 1;
  }

  if (video_metadata == nullptr ||
      !(video_metadata->audio || video_metadata->video)) {
    LOG(ERROR) << "The inputted video has unsupported format / "
                  "It does not have any audio/video tracks";
    return 1;
  }

  cli::Commands::Paths paths{video_path, output_subtitle_path};
  auto ffplay = std::make_unique<video::player::FFPlay>(
      FLAGS_ffplay_path, std::make_unique<subprocess::SubprocessExecutor>());
  auto wide_input_getter =
      std::make_unique<cli::io::WideInputGetter>(std::wcin);

  cli::Commands commands{paths, std::move(ffplay), std::move(wide_input_getter),
                         std::cout, std::move(video_metadata)};

  try {
    commands.MainLoop();
  } catch (const std::exception& e) {
    LOG(ERROR) << e.what();
    return 1;
  }

  return 0;
}
