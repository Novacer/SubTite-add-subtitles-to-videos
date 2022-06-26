#ifndef SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_MOCK_CLOUD_SERVICE_H
#define SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_MOCK_CLOUD_SERVICE_H

#include <gmock/gmock.h>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

/**
 * Mock class used to test speech-to-text cloud services.
 */
class MockCloudService : public STTCloudServiceBase {
  public:
    MOCK_METHOD(nlohmann::json, TranscribeBlocking,
                (const std::string&, std::function<void(const std::string&)>),
                (override));
};

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler

#endif
