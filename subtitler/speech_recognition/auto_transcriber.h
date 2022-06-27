#ifndef SUBTITLER_SPEECH_RECOGNITION_AUTO_TRANSCRIBER_H
#define SUBTITLER_SPEECH_RECOGNITION_AUTO_TRANSCRIBER_H

#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "subtitler/srt/subrip_file.h"

// Forward declaration
namespace subtitler::speech_recognition::cloud_service {

struct TranscriptionResult;
class STTCloudServiceBase;

}  // namespace subtitler::speech_recognition::cloud_service

namespace subtitler::speech_recognition::languages {

class Language;

}  // namespace subtitler::speech_recognition::languages

namespace subtitler {
namespace speech_recognition {

/**
 * Given an uncompressed audio wav file, produces a SRT subtitle file
 * with timestamps.
 */
class AutoTranscriber {
  public:
    AutoTranscriber(
        std::unique_ptr<cloud_service::STTCloudServiceBase> cloud_service,
        std::unique_ptr<languages::Language> language);
    ~AutoTranscriber();

    /**
     * Using the cloud service, obtain an SRT transcription of the audio.
     *
     * This function may run for a long time, so clients should run this in a
     * separate thread.
     *
     * @param input_wav the wav audio file path.
     * @param progress_msg_callback a callback to receive progress updates.
     * @return srt::SubRipFile the transcribed SRT file.
     */
    srt::SubRipFile Transcribe(
        const std::string input_wav,
        std::function<void(const std::string&)> progress_msg_callback);

  private:
    std::unique_ptr<cloud_service::STTCloudServiceBase> cloud_service_;
    std::unique_ptr<languages::Language> language_;

    srt::SubRipFile convertTranscriptionToSRT(
        const std::vector<cloud_service::TranscriptionResult>& transcriptions);
};

}  // namespace speech_recognition
}  // namespace subtitler

#endif
