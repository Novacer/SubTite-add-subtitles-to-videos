#include <gflags/gflags.h>
#include <glog/logging.h>
#include <speechapi_cxx.h>

#include <filesystem>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "subtitler/speech_recognition/auto_transcriber.h"
#include "subtitler/speech_recognition/cloud_service/microsoft_cognitive_service.h"
#include "subtitler/speech_recognition/languages/english_us.h"
#include "subtitler/srt/subrip_file.h"
#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/util/temp_file.h"
#include "subtitler/util/unicode.h"
#include "subtitler/video/processing/ffmpeg.h"

DEFINE_string(api_key, "",
              "Required. API Key of Microsoft.CognitiveServices.Speech.");

DEFINE_string(api_region, "westus",
              "Required. API region of Microsoft.CognitiveServices.Speech.");

namespace {

// Checks that the value of the flag is not empty string.
bool ValidateFlagNonEmpty(const char* flagname, const std::string& value) {
    return !value.empty();
}

}  // namespace

DEFINE_validator(api_key, &ValidateFlagNonEmpty);
DEFINE_validator(api_region, &ValidateFlagNonEmpty);

int main(int argc, char** argv) {
    using namespace Microsoft::CognitiveServices::Speech;
    using namespace Microsoft::CognitiveServices::Speech::Audio;

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

    using namespace subtitler;
    using namespace subtitler::speech_recognition;

    auto mcs_cloud_service =
        std::make_unique<cloud_service::MicrosoftCognitiveService>(
            FLAGS_api_key, FLAGS_api_region);
    auto english_us = std::make_unique<languages::EnglishUS>();
    auto auto_transcriber = std::make_unique<AutoTranscriber>(
        std::move(mcs_cloud_service), std::move(english_us));

    auto ffmpeg = std::make_unique<video::processing::FFMpeg>(
        "ffmpeg", std::make_unique<subprocess::SubprocessExecutor>());

    std::cout << "Please select the video file you want to auto transcribe."
              << std::endl;
    std::string video_path = "D:\\Videos\\patrick.mp4";

    TempFile temp{"", std::filesystem::path{video_path}.parent_path().string(),
                  ".wav"};

    ffmpeg->ExtractUncompressedAudio(video_path, temp.FileName());

    try {
        auto srt = auto_transcriber->Transcribe(
            temp.FileName(), [&](const std::string& msg) { LOG(INFO) << msg; });

        std::ostringstream output;
        srt.ToStream(output);

        LOG(INFO) << output.str();
    } catch (const std::exception& e) {
        LOG(ERROR) << e.what();
    }
}
