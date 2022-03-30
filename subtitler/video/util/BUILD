load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "video_utils",
    srcs = ["video_utils.cpp"],
    hdrs = ["video_utils.h"],
    deps = [] + select({
        "@bazel_tools//src/conditions:windows": [
            "@ffmpeg_windows//:ffmpeg_libavcodec",
            "@ffmpeg_windows//:ffmpeg_libavformat",
        ],
        "//conditions:default": [
            "@ffmpeg_linux//:ffmpeg_libavcodec",
            "@ffmpeg_linux//:ffmpeg_libavformat",
        ],
    }),
)
