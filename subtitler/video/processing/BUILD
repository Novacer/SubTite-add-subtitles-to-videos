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
