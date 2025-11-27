#ifndef SUBTITLER_SPEECH_RECOGNITION_LANGUAGES_ENGLISH_US_H
#define SUBTITLER_SPEECH_RECOGNITION_LANGUAGES_ENGLISH_US_H

#include <vector>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"
#include "subtitler/speech_recognition/languages/language.h"
#include "subtitler/srt/subrip_file.h"

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
      const std::vector<cloud_service::TranscriptionResult>& transcriptions)
      override;
};

}  // namespace languages
}  // namespace speech_recognition
}  // namespace subtitler

#endif
