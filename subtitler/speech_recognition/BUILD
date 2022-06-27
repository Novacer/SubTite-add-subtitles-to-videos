load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "auto_transcriber",
    srcs = ["auto_transcriber.cpp"],
    hdrs = ["auto_transcriber.h"],
    deps = [
        "//subtitler/speech_recognition/cloud_service:cloud_service_base",
        "//subtitler/srt:subrip_file",
        "@com_github_nlohmann_json//:json",
    ],
)

# Test using microsoft cognitive services.
cc_test(
    name = "auto_transcriber_test_mcs",
    srcs = ["auto_transcriber_test_mcs.cpp"],
    data = ["//test_data:mcs_speech_to_text"],
    target_compatible_with = [
        "@platforms//os:windows",
    ],
    deps = [
        ":auto_transcriber",
        "//subtitler/speech_recognition/cloud_service:mock_microsoft_cognitive_service",
        "//test_data:bazel_utils",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest_main",
    ],
)
