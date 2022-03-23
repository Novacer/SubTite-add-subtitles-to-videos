load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler/cli:__subpackages__"])

cc_library(
    name = "input",
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["input_msvc.cpp"],
        "//conditions:default": ["input_gcc.cpp"],
    }),
    hdrs = ["input.h"],
    deps = [
        "//subtitler/util:unicode",
    ],
)
