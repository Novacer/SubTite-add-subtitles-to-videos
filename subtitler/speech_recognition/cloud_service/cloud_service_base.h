#ifndef SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_CLOUD_SERVICE_BASE_H
#define SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_CLOUD_SERVICE_BASE_H

#include <functional>
#include <nlohmann/json.hpp>
#include <string>

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

// Abstract Interface for a speech-to-text cloud service.
class STTCloudServiceBase {
  public:
    STTCloudServiceBase() = default;
    virtual ~STTCloudServiceBase() = default;

    virtual nlohmann::json TranscribeBlocking(
        const std::string& input_wav,
        std::function<void(const std::string&)> progress_msg_callback) = 0;
};

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler

#endif
