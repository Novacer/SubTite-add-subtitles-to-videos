#include "subtitler/speech_recognition/auto_transcriber.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"
#include "subtitler/speech_recognition/cloud_service/mock_cloud_service.h"

using subtitler::speech_recognition::AutoTranscriber;
using subtitler::speech_recognition::cloud_service::MockCloudService;
using subtitler::speech_recognition::cloud_service::TranscriptionResult;
using ::testing::_;
using ::testing::IsEmpty;
using ::testing::Return;

TEST(AutoTranscriber, SanityCheck) {
    auto mock_service = std::make_unique<MockCloudService>();
    std::function<void(const std::string&)> callback =
        [](const std::string& x) {};
    std::string input_wav = "input.wav";
    const auto mock_transcript = TranscriptionResult{};

    EXPECT_CALL(*mock_service, TranscribeBlocking(input_wav, _))
        .Times(1)
        .WillOnce(Return(mock_transcript));
    
    AutoTranscriber transcriber{std::move(mock_service)};
    auto result = transcriber.Transcribe(input_wav, callback);
    ASSERT_EQ(result.NumItems(), 0);
}
