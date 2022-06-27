#include "subtitler/speech_recognition/languages/english_us.h"

#include <chrono>
#include <sstream>
#include <stdexcept>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"
#include "subtitler/srt/subrip_item.h"

namespace subtitler {
namespace speech_recognition {
namespace languages {

srt::SubRipFile EnglishUS::ConvertToSRT(
    const cloud_service::TranscriptionResult& transcription) {
    srt::SubRipFile srt_file;

    std::istringstream tokenizer{transcription.display_text};

    std::string sentence_so_far;
    std::string next_word;
    unsigned int word_index = 0;
    std::chrono::milliseconds start;

    while (tokenizer >> next_word) {
        if (next_word.empty()) {
            throw std::runtime_error{"Tokenizer gave unexpected empty word"};
        }
        if (transcription.timings.empty()) {
            throw std::runtime_error{"Transcription did not have word timings"};
        }
        if (sentence_so_far.empty()) {
            start = transcription.timings.at(word_index).offset;
        } else {
            sentence_so_far += ' ';
        }

        sentence_so_far += next_word;

        bool sentence_ends = next_word.back() == '!' ||
                             next_word.back() == '?' || next_word.back() == '.';
        bool no_more_word_timings =
            word_index + 1 >= transcription.timings.size();

        if (sentence_ends || no_more_word_timings) {
            const auto& timing = transcription.timings.at(word_index);
            const auto end = timing.offset + timing.duration;
            srt::SubRipItem item{};
            item.start(start)
                ->duration(end - start)
                ->AppendLine(sentence_so_far);
            srt_file.AddItem(std::move(item));
            sentence_so_far.clear();

            if (no_more_word_timings) {
                break;
            }
        }
        ++word_index;
    }

    return srt_file;
}

}  // namespace languages
}  // namespace speech_recognition
}  // namespace subtitler
