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

cc_test(
    name = "progress_parser_test",
    size = "small",
    srcs = ["progress_parser_test.cpp"],
    deps = [
        ":progress_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "downscaling",
    srcs = ["downscaling.cpp"],
    hdrs = ["downscaling.h"],
    deps = [],
)

cc_test(
    name = "downscaling_test",
    size = "small",
    srcs = ["downscaling_test.cpp"],
    deps = [
        ":downscaling",
        "@com_google_googletest//:gtest_main",
    ],
)
