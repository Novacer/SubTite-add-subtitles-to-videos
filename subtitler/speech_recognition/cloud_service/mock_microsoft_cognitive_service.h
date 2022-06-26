#ifndef SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_MOCK_MICROSOFT_COGNITIVE_SERVICE_H
#define SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_MOCK_MICROSOFT_COGNITIVE_SERVICE_H

#include <gmock/gmock.h>

#include <nlohmann/json.hpp>

#include "subtitler/speech_recognition/cloud_service/microsoft_cognitive_service.h"

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

/**
 * Mock class used to test speech-to-text cloud services.
 */
class MockMicrosoftCognitiveService : public MicrosoftCognitiveService {
  public:
    MockMicrosoftCognitiveService()
        : MicrosoftCognitiveService{"api-key", "api-region"} {};

    MOCK_METHOD(nlohmann::json, getTranscriptionJson,
                (const std::string&, std::function<void(const std::string&)>),
                (override));
};

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler

#endif
