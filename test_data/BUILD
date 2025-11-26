load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "bazel_utils",
    testonly = True,
    srcs = ["bazel_utils.cpp"],
    hdrs = ["bazel_utils.h"],
    deps = ["@bazel_tools//tools/cpp/runfiles"],
)

filegroup(
    name = "mcs_speech_to_text",
    testonly = True,
    srcs = [
        "mcs_speech_to_text.json",
    ],
    data = [
        "mcs_speech_to_text.json",
    ],
)
