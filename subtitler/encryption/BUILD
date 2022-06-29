load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "file_encryption",
    srcs = ["file_encryption.cpp"],
    hdrs = ["file_encryption.h"],
    linkopts = select({
        "@bazel_tools//src/conditions:windows": [""],
        "//conditions:default": ["-lsodium"],
    }),
    deps = select({
        "@bazel_tools//src/conditions:windows": ["@libsodium_windows//:libsodium"],
        "//conditions:default": [],
    }),
)

cc_test(
    name = "file_encryption_test",
    srcs = ["file_encryption_test.cpp"],
    deps = [
        ":file_encryption",
        "//subtitler/util:temp_file",
        "@com_google_googletest//:gtest_main",
    ],
)
