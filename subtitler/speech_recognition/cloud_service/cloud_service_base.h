#ifndef SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_CLOUD_SERVICE_BASE_H
#define SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_CLOUD_SERVICE_BASE_H

#include <functional>
#include <string>
#include <chrono>

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

struct WordTimings {
  std::string word;
  std::chrono::milliseconds offset;
  std::chrono::milliseconds duration;
};

struct TranscriptionResult {
  std::string display_text;
  std::vector<WordTimings> timings;
};

// Abstract Interface for a speech-to-text cloud service.
class STTCloudServiceBase {
  public:
    STTCloudServiceBase() = default;
    virtual ~STTCloudServiceBase() = default;

    virtual TranscriptionResult TranscribeBlocking(
        const std::string& input_wav,
        std::function<void(const std::string&)> progress_msg_callback) = 0;
};

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler

#endif
