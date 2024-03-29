#ifndef SUBTITLER_SPEECH_RECOGNITION_LANGUAGES_LANGUAGE_H
#define SUBTITLER_SPEECH_RECOGNITION_LANGUAGES_LANGUAGE_H

#include <vector>

#include "subtitler/srt/subrip_file.h"

// Forward declaration
namespace subtitler::speech_recognition::cloud_service {

struct TranscriptionResult;

}  // namespace subtitler::speech_recognition::cloud_service

namespace subtitler {
namespace speech_recognition {
namespace languages {

/**
 * Interface on how TranscriptionResult shall be converted to SRT for different
 * languages. For example, it defines how words should be grouped into
 * sentences.
 */
class Language {
  public:
    Language() = default;
    virtual ~Language() = default;

    // Derived classes must override.
    virtual srt::SubRipFile ConvertToSRT(
        const std::vector<cloud_service::TranscriptionResult>&
            transcriptions) = 0;
};

}  // namespace languages
}  // namespace speech_recognition
}  // namespace subtitler

#endif
