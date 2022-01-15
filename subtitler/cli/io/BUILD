load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler/cli:__subpackages__"])

cc_library(
    name = "input",
    hdrs = ["input.h"],
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["input_msvc.cpp"],
        "@bazel_tools//src/conditions:darwin": ["input_gcc.cpp"],
        "//conditions:default": ["input_gcc.cpp"],
    }),
    deps = [
        "//subtitler/util:unicode",
    ],
)
