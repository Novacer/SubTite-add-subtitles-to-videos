#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>

#include "subtitler/speech_recognition/cloud_service/mock_microsoft_cognitive_service.h"
#include "test_data/bazel_utils.h"

using subtitler::speech_recognition::cloud_service::
    MockMicrosoftCognitiveService;
using subtitler::speech_recognition::cloud_service::TranscriptionResult;
using ::testing::_;
using ::testing::Return;
using ::testing::SizeIs;
using namespace std::chrono_literals;

TEST(MicrosoftCognitiveService, ParseJson) {
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

    auto result =
        mock_service->TranscribeBlocking(input_wav, [](const std::string&) {});

    EXPECT_EQ(
        result.display_text,
        "You may be an open book Sponge Bob, but I'm a bit more complicated "
        "than that. The inner machinations of my mind are an enigma.");
    EXPECT_EQ(result.initial_offset, 80ms);
    // Total duration should be around 10s.
    EXPECT_EQ(result.total_duration, 10540ms);
    EXPECT_THAT(result.timings, SizeIs(25));
}
