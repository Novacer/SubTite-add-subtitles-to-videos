#include <gflags/gflags.h>
#include <glog/logging.h>
#include <speechapi_cxx.h>

#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

DEFINE_string(api_key, "",
              "Required. API Key of Microsoft.CognitiveServices.Speech.");

DEFINE_string(api_region, "westus",
              "Required. API region of Microsoft.CognitiveServices.Speech.");

namespace {

// Checks that the value of the flag is not empty string.
bool ValidateFlagNonEmpty(const char* flagname, const std::string& value) {
    return !value.empty();
}

}  // namespace

DEFINE_validator(api_key, &ValidateFlagNonEmpty);
DEFINE_validator(api_region, &ValidateFlagNonEmpty);

int main(int argc, char** argv) {
    using namespace Microsoft::CognitiveServices::Speech;
    using namespace Microsoft::CognitiveServices::Speech::Audio;

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

    auto speechConfig =
        SpeechConfig::FromSubscription(FLAGS_api_key, FLAGS_api_region);
    speechConfig->SetSpeechRecognitionLanguage("en-US");
    speechConfig->RequestWordLevelTimestamps();

    auto audioInput = AudioConfig::FromWavFileInput(
        "C:\\Users\\zhang\\Videos\\patrick\\patrick.wav");
    auto recognizer = SpeechRecognizer::FromConfig(speechConfig, audioInput);
    std::promise<void> recognitionEnd;

    recognizer->Recognized.Connect([](const SpeechRecognitionEventArgs& e) {
        if (e.Result->Reason == ResultReason::RecognizedSpeech) {
            auto start = e.Result->Offset();
            auto end = start + e.Result->Duration();
            LOG(INFO) << "RECOGNIZED: Text=" << e.Result->Text
                      << " start: " << start << " end: " << end;

            auto result =
                nlohmann::json::parse(e.Result->Properties.GetProperty(
                    PropertyId::SpeechServiceResponse_JsonResult));

            LOG(INFO) << result.dump(2);
        } else if (e.Result->Reason == ResultReason::NoMatch) {
            LOG(INFO) << "NOMATCH: Speech could not be recognized.";
        }
    });

    recognizer->Canceled.Connect(
        [&recognitionEnd](const SpeechRecognitionCanceledEventArgs& e) {
            LOG(INFO) << "CANCELED: Reason=" << (int)e.Reason;
            if (e.Reason == CancellationReason::Error) {
                LOG(INFO)
                    << "CANCELED: ErrorCode=" << (int)e.ErrorCode << "\n"
                    << "CANCELED: ErrorDetails=" << e.ErrorDetails << "\n"
                    << "CANCELED: Did you set the speech resource key and "
                       "region values?";

                recognitionEnd.set_value();  // Notify to stop recognition.
            }
        });

    recognizer->SessionStopped.Connect(
        [&recognitionEnd](const SessionEventArgs& e) {
            LOG(INFO) << "Session stopped.";
            recognitionEnd.set_value();  // Notify to stop recognition.
        });

    recognizer->StartContinuousRecognitionAsync().get();
    recognitionEnd.get_future().get();
    recognizer->StopContinuousRecognitionAsync().get();
}
