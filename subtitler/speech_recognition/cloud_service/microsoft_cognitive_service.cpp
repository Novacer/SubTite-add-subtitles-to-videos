#include "subtitler/speech_recognition/cloud_service/microsoft_cognitive_service.h"

#include <speechapi_cxx.h>

#include <stdexcept>

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

namespace {

std::chrono::milliseconds ticksToMs(int64_t ticks) {
    // 1 tick = 100 ns.
    // 1 tick / 100 ns = 1000000 ns / 1 ms.
    return std::chrono::milliseconds{ticks / 10000};
}

}  // namespace

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
    // TODO
    return {};
}

TranscriptionResult MicrosoftCognitiveService::parseJson(
    const nlohmann::json& json) {
    TranscriptionResult result;

    result.display_text = json.at("DisplayText").get<std::string>();
    result.initial_offset = ticksToMs(json.at("Offset").get<int64_t>());
    result.total_duration = ticksToMs(json.at("Duration").get<int64_t>());

    const auto& best_guess = json.at("NBest").at(0);
    const auto& word_timings = best_guess.at("Words");

    for (const auto& timing : word_timings) {
        const auto offset_ms = ticksToMs(timing.at("Offset").get<int64_t>());
        const auto duration_ms =
            ticksToMs(timing.at("Duration").get<int64_t>());
        result.timings.push_back(WordTimings{
            timing.at("Word").get<std::string>(), offset_ms, duration_ms});
    }

    return result;
}

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler
