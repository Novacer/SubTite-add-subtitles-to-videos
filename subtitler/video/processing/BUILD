load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "ffmpeg",
    srcs = ["ffmpeg.cpp"],
    hdrs = ["ffmpeg.h"],
    deps = [
        ":progress_parser",
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/video/util:video_utils",
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

cc_library(
    name = "progress_parser",
    srcs = ["progress_parser.cpp"],
    hdrs = ["progress_parser.h"],
    deps = [],
)
