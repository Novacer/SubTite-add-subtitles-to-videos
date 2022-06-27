#include "subtitler/speech_recognition/auto_transcriber.h"

#include <stdexcept>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"
#include "subtitler/speech_recognition/languages/language.h"

namespace subtitler {
namespace speech_recognition {

AutoTranscriber::AutoTranscriber(
    std::unique_ptr<cloud_service::STTCloudServiceBase> cloud_service,
    std::unique_ptr<languages::Language> language)
    : cloud_service_{std::move(cloud_service)}, language_{std::move(language)} {
    if (!cloud_service_) {
        throw std::invalid_argument{"cloud service cannot be null"};
    }
    if (!language_) {
        throw std::invalid_argument{"language cannot be null"};
    }
}

AutoTranscriber::~AutoTranscriber() = default;

srt::SubRipFile AutoTranscriber::Transcribe(
    const std::string input_wav,
    std::function<void(const std::string&)> progress_msg_callback) {
    const auto transcription =
        cloud_service_->TranscribeBlocking(input_wav, progress_msg_callback);
    return convertTranscriptionToSRT(transcription);
}

srt::SubRipFile AutoTranscriber::convertTranscriptionToSRT(
    const std::vector<cloud_service::TranscriptionResult>& transcriptions) {
    return language_->ConvertToSRT(transcriptions);
}

}  // namespace speech_recognition
}  // namespace subtitler
