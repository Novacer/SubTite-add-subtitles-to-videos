#ifndef SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_MICROSOFT_COGNITIVE_SERVICE_H
#define SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_MICROSOFT_COGNITIVE_SERVICE_H

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

/**
 * Using
 * https://docs.microsoft.com/en-us/azure/cognitive-services/Speech-Service/
 */
class MicrosoftCognitiveService : public STTCloudServiceBase {
 public:
  MicrosoftCognitiveService(std::string_view api_key,
                            std::string_view api_region);
  virtual ~MicrosoftCognitiveService() = default;

 protected:
  std::vector<nlohmann::json> getTranscriptionJson(
      const std::string& input_wav,
      std::function<void(const std::string&)> progress_msg_callback) override;

  std::vector<TranscriptionResult> parseJson(
      const std::vector<nlohmann::json>& jsons) override;

 private:
  const std::string api_key_;
  const std::string api_region_;
};

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler

#endif
