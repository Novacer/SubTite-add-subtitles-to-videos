#include "subtitler/speech_recognition/cloud_service/microsoft_cognitive_service.h"

#include <speechapi_cxx.h>

#include <stdexcept>
#include <string_view>

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
    const std::string_view api_key, const std::string_view api_region)
    : STTCloudServiceBase{}, api_key_{api_key}, api_region_{api_region} {
    if (api_key_.empty()) {
        throw std::invalid_argument{"API Key cannot be empty!"};
    }
    if (api_region_.empty()) {
        throw std::invalid_argument{"API Region cannot be empty! Ex: westus"};
    }
}

std::vector<nlohmann::json> MicrosoftCognitiveService::getTranscriptionJson(
    const std::string& input_wav,
    std::function<void(const std::string&)> progress_msg_callback) {
    using namespace Microsoft::CognitiveServices::Speech;
    using namespace Microsoft::CognitiveServices::Speech::Audio;

    auto speech_config = SpeechConfig::FromSubscription(api_key_, api_region_);
    // TODO: in the future we want to support other languages also.
    speech_config->SetSpeechRecognitionLanguage("en-US");
    speech_config->RequestWordLevelTimestamps();

    auto audio_input = AudioConfig::FromWavFileInput(input_wav);
    auto recognizer = SpeechRecognizer::FromConfig(speech_config, audio_input);
    std::promise<void> recognition_end;

    std::vector<nlohmann::json> jsons;

    recognizer->Recognized.Connect(
        [&](const SpeechRecognitionEventArgs& event) {
            if (event.Result->Reason == ResultReason::RecognizedSpeech) {
                // If there is no speech in the audio, then API does not
                // return any words. Filter out those segments here.
                if (event.Result->Text.empty()) {
                    return;
                }

                auto json =
                    nlohmann::json::parse(event.Result->Properties.GetProperty(
                        PropertyId::SpeechServiceResponse_JsonResult));
                jsons.push_back(std::move(json));

                progress_msg_callback("RECOGNIZED: Text=" + event.Result->Text);
            } else if (event.Result->Reason == ResultReason::NoMatch) {
                progress_msg_callback(
                    "NOMATCH: Speech could not be recognized");
            }
        });

    recognizer->Canceled.Connect(
        [&](const SpeechRecognitionCanceledEventArgs& event) {
            if (event.Reason == CancellationReason::Error) {
                progress_msg_callback("CANCELED: ErrorDetails=" +
                                      event.ErrorDetails);

                recognition_end.set_value();  // Notify to stop recognition.
            }
        });

    recognizer->SessionStopped.Connect([&](const SessionEventArgs& e) {
        progress_msg_callback("Finished");
        recognition_end.set_value();  // Notify to stop recognition.
    });

    recognizer->StartContinuousRecognitionAsync().get();
    recognition_end.get_future().get();
    recognizer->StopContinuousRecognitionAsync().get();

    return jsons;
}

std::vector<TranscriptionResult> MicrosoftCognitiveService::parseJson(
    const std::vector<nlohmann::json>& jsons) {
    std::vector<TranscriptionResult> results;

    for (const auto& json : jsons) {
        TranscriptionResult result;

        result.display_text = json.at("DisplayText").get<std::string>();
        result.initial_offset = ticksToMs(json.at("Offset").get<int64_t>());
        result.total_duration = ticksToMs(json.at("Duration").get<int64_t>());

        const auto& best_guess = json.at("NBest").at(0);
        const auto& word_timings = best_guess.at("Words");

        for (const auto& timing : word_timings) {
            const auto offset_ms =
                ticksToMs(timing.at("Offset").get<int64_t>());
            const auto duration_ms =
                ticksToMs(timing.at("Duration").get<int64_t>());
            result.timings.push_back(WordTimings{
                timing.at("Word").get<std::string>(), offset_ms, duration_ms});
        }

        results.push_back(std::move(result));
    }

    return results;
}

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler
