#ifndef SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_MICROSOFT_COGNITIVE_SERVICE_H
#define SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_MICROSOFT_COGNITIVE_SERVICE_H

#include <nlohmann/json.hpp>
#include <string>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

class MicrosoftCognitiveService : public STTCloudServiceBase {
  public:
    MicrosoftCognitiveService(const std::string& api_key,
                              const std::string& api_region);
    virtual ~MicrosoftCognitiveService() = default;

    nlohmann::json TranscribeBlocking(
        const std::string& input_wav,
        std::function<void(const std::string&)> progress_msg_callback) override;

  private:
    const std::string api_key_;
    const std::string api_region_;
};

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler

#endif
