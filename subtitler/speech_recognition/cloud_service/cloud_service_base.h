#ifndef SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_CLOUD_SERVICE_BASE_H
#define SUBTITLER_SPEECH_RECOGNITION_CLOUD_SERVICE_CLOUD_SERVICE_BASE_H

#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>

namespace subtitler {
namespace speech_recognition {
namespace cloud_service {

/**
 * Info on word level timings.
 */
struct WordTimings {
    std::string word;
    // The time in the audio where the word starts.
    std::chrono::milliseconds offset;
    // The the amount of time the word is spoken for.
    std::chrono::milliseconds duration;
};

/**
 * The transcription of the audio with word-level timings.
 */
struct TranscriptionResult {
    // The spoken text, often with punctuation.
    std::string display_text;
    // The list of words and their timings.
    std::vector<WordTimings> timings;
    // The time in the audio this transcription starts.
    std::chrono::milliseconds initial_offset;
    // The duration in the audio this transcription contains.
    std::chrono::milliseconds total_duration;
};

// Abstract Interface for a speech-to-text cloud service.
class STTCloudServiceBase {
  public:
    STTCloudServiceBase() = default;
    virtual ~STTCloudServiceBase() = default;

    /**
     * Given an input wav audio file, upload to a Speech-To-Text cloud service
     * and get the transcript with word-level timestamps.
     *
     * This function likely blocks for a long time during upload, thus clients
     * should run this in a separate thread.
     *
     * @param input_wav the filename of the input audio file.
     * @param progress_msg_callback callback to receive progress updates.
     * @return TranscriptionResult the transcription from the cloud service.
     */
    TranscriptionResult TranscribeBlocking(
        const std::string& input_wav,
        std::function<void(const std::string&)> progress_msg_callback) {
        return parseJson(
            getTranscriptionJson(input_wav, progress_msg_callback));
    }

  protected:
    /**
     * Given the input audio file, get the json response from the cloud service.
     * Derived class must implement this.
     *
     * @param input_wav the audio file path.
     * @param progress_msg_callback a callback to push progress updates.
     * @return nlohmann::json the raw json response from the service.
     */
    virtual nlohmann::json getTranscriptionJson(
        const std::string& input_wav,
        std::function<void(const std::string&)> progress_msg_callback) = 0;

    /**
     * Given the raw json response from the service, parse it into the
     * standardized TranscriptionResult with word-level timings.
     * Derived class must implement this.
     *
     * @param json the json response from the server.
     * @return TranscriptionResult the standardized transcription result.
     */
    virtual TranscriptionResult parseJson(const nlohmann::json& json) = 0;
};

}  // namespace cloud_service
}  // namespace speech_recognition
}  // namespace subtitler

#endif
