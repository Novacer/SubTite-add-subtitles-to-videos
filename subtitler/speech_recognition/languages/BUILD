load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "language",
    hdrs = ["language.h"],
    deps = [
        "//subtitler/srt:subrip_file",
    ],
)

cc_library(
    name = "english_us",
    srcs = ["english_us.cpp"],
    hdrs = ["english_us.h"],
    deps = [
        ":language",
        "//subtitler/speech_recognition/cloud_service:cloud_service_base",
        "//subtitler/srt:subrip_file",
        "//subtitler/srt:subrip_item",
    ],
)

cc_test(
    name = "english_us_test",
    srcs = ["english_us_test.cpp"],
    deps = [
        ":english_us",
        "//subtitler/speech_recognition/cloud_service:cloud_service_base",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest_main",
    ],
)
