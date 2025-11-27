#include "subtitler/speech_recognition/languages/english_us.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <sstream>
#include <vector>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"

using subtitler::speech_recognition::cloud_service::TranscriptionResult;
using subtitler::speech_recognition::cloud_service::WordTimings;
using subtitler::speech_recognition::languages::EnglishUS;
using subtitler::srt::SubRipFile;
using ::testing::_;
using namespace std::chrono_literals;

TEST(EnglishUS, SplitSentences) {
  EnglishUS english;
  TranscriptionResult transcription;
  transcription.display_text = "Hello, world! Are you there? Yes.";
  transcription.initial_offset = 0ms;
  transcription.total_duration = 10s;

  std::vector<WordTimings> timings = {
      WordTimings{/* word= */ "hello",
                  /* offset= */ 0ms,
                  /* duration= */ 1s},
      WordTimings{/* word= */ "world",
                  /* offset= */ 1s + 100ms,
                  /* duration= */ 2s},
      WordTimings{/* word= */ "are",
                  /* offset= */ 4s,
                  /* duration= */ 500ms},
      WordTimings{/* word= */ "you",
                  /* offset= */ 5s,
                  /* duration= */ 1s},
      WordTimings{/* word= */ "there",
                  /* offset= */ 6s,
                  /* duration= */ 300ms},
      WordTimings{/* word= */ "yes",
                  /* offset= */ 8s,
                  /* duration= */ 2s},
  };
  transcription.timings = std::move(timings);

  auto srt_file =
      english.ConvertToSRT(std::vector<TranscriptionResult>{transcription});
  std::ostringstream output;
  srt_file.ToStream(output);

  EXPECT_EQ(output.str(),
            "1\n"
            "00:00:00,000 --> 00:00:03,100\n"
            "Hello, world!\n"
            "\n"
            "2\n"
            "00:00:04,000 --> 00:00:06,300\n"
            "Are you there?\n"
            "\n"
            "3\n"
            "00:00:08,000 --> 00:00:10,000\n"
            "Yes.\n"
            "\n");
}

TEST(EnglishUS, EmptySentenceReturnsEmptyFile) {
  EnglishUS english;
  TranscriptionResult transcription;
  transcription.display_text = "  \t\r\n";
  transcription.initial_offset = 0ms;
  transcription.total_duration = 10s;

  auto srt_file =
      english.ConvertToSRT(std::vector<TranscriptionResult>{transcription});

  EXPECT_EQ(srt_file.NumItems(), 0);
}

TEST(EnglishUS, NoSentenceEnderReturnsOneSentence) {
  EnglishUS english;
  TranscriptionResult transcription;
  transcription.display_text = "Hello, world";
  transcription.initial_offset = 0ms;
  transcription.total_duration = 4s;

  std::vector<WordTimings> timings = {
      WordTimings{/* word= */ "hello",
                  /* offset= */ 0ms,
                  /* duration= */ 1s},
      WordTimings{/* word= */ "world",
                  /* offset= */ 1s + 100ms,
                  /* duration= */ 2s},
  };
  transcription.timings = std::move(timings);

  auto srt_file =
      english.ConvertToSRT(std::vector<TranscriptionResult>{transcription});
  std::ostringstream output;
  srt_file.ToStream(output);

  EXPECT_EQ(output.str(),
            "1\n"
            "00:00:00,000 --> 00:00:03,100\n"
            "Hello, world\n"
            "\n");
}
