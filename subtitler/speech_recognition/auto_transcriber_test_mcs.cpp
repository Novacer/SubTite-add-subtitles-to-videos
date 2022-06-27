#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>

#include "subtitler/speech_recognition/auto_transcriber.h"
#include "subtitler/speech_recognition/cloud_service/mock_microsoft_cognitive_service.h"
#include "test_data/bazel_utils.h"

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

    std::ifstream json_data{test_data::GetBazelDataAbsolutePath(
        "__main__/test_data/mcs_speech_to_text.json")};

    if (!json_data) {
        FAIL() << "Failed to open json_data";
    }
    nlohmann::json mock_response;
    if (!(json_data >> mock_response)) {
        FAIL() << "Could not read json_data";
    }

    EXPECT_CALL(*mock_service, getTranscriptionJson(input_wav, _))
        .Times(1)
        .WillOnce(Return(mock_response));

    AutoTranscriber transcriber{std::move(mock_service)};
    auto result = transcriber.Transcribe(input_wav, callback);
    ASSERT_EQ(result.NumItems(), 0);
}
