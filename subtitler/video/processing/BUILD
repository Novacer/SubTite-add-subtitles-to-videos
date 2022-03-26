load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "ffmpeg",
    srcs = ["ffmpeg.cpp"],
    hdrs = ["ffmpeg.h"],
    deps = [
        "//subtitler/subprocess:subprocess_executor",
    ],
)

cc_test(
    name = "ffmpeg_test",
    size = "small",
    srcs = ["ffmpeg_test.cpp"],
    deps = [
        ":ffmpeg",
        "//subtitler/subprocess:mock_subprocess_executor",
        "@com_google_googletest//:gtest_main",
    ],
)