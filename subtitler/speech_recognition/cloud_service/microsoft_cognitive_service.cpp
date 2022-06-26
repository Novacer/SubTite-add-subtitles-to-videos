#include "subtitler/speech_recognition/cloud_service/microsoft_cognitive_service.h"

#include <stdexcept>

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

MicrosoftCognitiveService::MicrosoftCognitiveService(
    const std::string& api_key, const std::string& api_region)
    : STTCloudServiceBase{}, api_key_{api_key}, api_region_{api_region} {
    if (api_key_.empty()) {
        throw std::invalid_argument{"API Key cannot be empty!"};
    }
    if (api_region_.empty()) {
        throw std::invalid_argument{"API Region cannot be empty! Ex: westus"};
    }
}

nlohmann::json MicrosoftCognitiveService::getTranscriptionJson(
    const std::string& input_wav,
    std::function<void(const std::string&)> progress_msg_callback) {
    return {};
}

TranscriptionResult MicrosoftCognitiveService::parseJson(
    const nlohmann::json& json) {
    return {};
}

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler
