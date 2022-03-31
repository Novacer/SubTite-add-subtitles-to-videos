load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "video_utils",
    srcs = ["video_utils.cpp"],
    hdrs = ["video_utils.h"],
    deps = [] + select({
        "@bazel_tools//src/conditions:windows": [
            "@ffmpeg_windows//:ffmpeg_libavcodec",
            "@ffmpeg_windows//:ffmpeg_libavdevice",
            "@ffmpeg_windows//:ffmpeg_libavfilter",
            "@ffmpeg_windows//:ffmpeg_libavformat",
            "@ffmpeg_windows//:ffmpeg_libavutil",
            "@ffmpeg_windows//:ffmpeg_libswresample",
            "@ffmpeg_windows//:ffmpeg_libswscale",
        ],
        "//conditions:default": [
            "@ffmpeg_linux//:ffmpeg_libavcodec",
            "@ffmpeg_linux//:ffmpeg_libavdevice",
            "@ffmpeg_linux//:ffmpeg_libavfilter",
            "@ffmpeg_linux//:ffmpeg_libavformat",
            "@ffmpeg_linux//:ffmpeg_libavutil",
            "@ffmpeg_linux//:ffmpeg_libswresample",
            "@ffmpeg_linux//:ffmpeg_libswscale",
        ],
    }),
)
