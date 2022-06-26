#ifndef SUBTITLER_SPEECH_RECOGNITION_AUTO_TRANSCRIBER_H
#define SUBTITLER_SPEECH_RECOGNITION_AUTO_TRANSCRIBER_H

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "subtitler/srt/subrip_file.h"

// Forward declaration
namespace subtitler::speech_recognition::cloud_service {

class STTCloudServiceBase;

}

namespace subtitler {
namespace speech_recognition {

class AutoTranscriber {
  public:
    AutoTranscriber(
        std::unique_ptr<cloud_service::STTCloudServiceBase> cloud_service);
    ~AutoTranscriber();

    // Long running function, should be run in separate thread.
    srt::SubRipFile Transcribe(
        const std::string& input_wav,
        std::function<void(const std::string&)> progress_msg_callback);

  private:
    std::unique_ptr<cloud_service::STTCloudServiceBase> cloud_service_;

    srt::SubRipFile parseJsonResult(const nlohmann::json& result);
};

}  // namespace speech_recognition
}  // namespace subtitler

#endif
