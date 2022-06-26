#include "subtitler/speech_recognition/auto_transcriber.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <nlohmann/json.hpp>

#include "subtitler/speech_recognition/cloud_service/cloud_service_base.h"
#include "subtitler/speech_recognition/cloud_service/mock_microsoft_cognitive_service.h"

using subtitler::speech_recognition::AutoTranscriber;
using subtitler::speech_recognition::cloud_service::
    MockMicrosoftCognitiveService;
using subtitler::speech_recognition::cloud_service::TranscriptionResult;
using ::testing::_;
using ::testing::IsEmpty;
using ::testing::Return;

TEST(AutoTranscriber, UsingMicrosoftCongitiveService) {
    auto mock_service = std::make_unique<MockMicrosoftCognitiveService>();
    std::function<void(const std::string&)> callback =
        [](const std::string& x) {};
    std::string input_wav = "input.wav";
    const auto mock_json = "{ \"key\" : \"value\" }"_json;

    EXPECT_CALL(*mock_service, getTranscriptionJson(input_wav, _))
        .Times(1)
        .WillOnce(Return(mock_json));

    AutoTranscriber transcriber{std::move(mock_service)};
    auto result = transcriber.Transcribe(input_wav, callback);
    ASSERT_EQ(result.NumItems(), 0);
}
