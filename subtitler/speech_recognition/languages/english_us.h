#ifndef SUBTITLER_SPEECH_RECOGNITION_LANGUAGES_ENGLISH_US_H
#define SUBTITLER_SPEECH_RECOGNITION_LANGUAGES_ENGLISH_US_H

#include "subtitler/speech_recognition/languages/language.h"
#include "subtitler/srt/subrip_file.h"

// Forward declaration
namespace subtitler::speech_recognition::cloud_service {

struct TranscriptionResult;

}

namespace subtitler {
namespace speech_recognition {
namespace languages {

/**
 * Defines how TranscriptionResults in English US are to be converted
 * to SRT format.
 */
class EnglishUS final : public Language {
  public:
    EnglishUS() = default;
    ~EnglishUS() = default;

    srt::SubRipFile ConvertToSRT(
        const cloud_service::TranscriptionResult& transcription) override;
};

}  // namespace languages
}  // namespace speech_recognition
}  // namespace subtitler

#endif
