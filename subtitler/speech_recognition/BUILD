load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "auto_transcriber",
    srcs = ["auto_transcriber.cpp"],
    hdrs = ["auto_transcriber.h"],
    target_compatible_with = [
        "@platforms//os:windows",
    ],
    deps = [
        "//subtitler/speech_recognition/cloud_service:cloud_service_base",
        "//subtitler/srt:subrip_file",
        "@com_github_nlohmann_json//:json",
    ],
)

cc_test(
    name = "auto_transcriber_test",
    srcs = ["auto_transcriber_test.cpp"],
    deps = [
        ":auto_transcriber",
        "//subtitler/speech_recognition/cloud_service:mock_cloud_service",
        "@com_google_googletest//:gtest_main",
    ],
)
