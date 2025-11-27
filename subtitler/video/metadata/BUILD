load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "ffprobe",
    srcs = ["ffprobe.cpp"],
    hdrs = ["ffprobe.h"],
    deps = [
        "//subtitler/subprocess:subprocess_executor",
        "@com_github_nlohmann_json//:json",
    ],
)

cc_test(
    name = "ffprobe_test",
    size = "small",
    srcs = ["ffprobe_test.cpp"],
    deps = [
        ":ffprobe",
        "//subtitler/subprocess:mock_subprocess_executor",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest_main",
    ],
)
