load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "duration_format",
    srcs = ["duration_format.cpp"],
    hdrs = ["duration_format.h"],
    deps = [
        "@howard_hinnant_date//:date",
    ],
)

cc_test(
    name = "duration_format_test",
    size = "small",
    srcs = ["duration_format_test.cpp"],
    deps = [
        ":duration_format",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "font_config",
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["font_config_msvc.cpp"],
        "//conditions:default": ["font_config_gcc.cpp"],
    }),
    hdrs = ["font_config.h"],
    deps = [],
)

cc_library(
    name = "temp_file",
    srcs = ["temp_file.cpp"],
    hdrs = ["temp_file.h"],
    deps = [
        ":unicode",
        "@com_github_google_glog//:glog",
    ],
)

cc_test(
    name = "temp_file_test",
    size = "small",
    srcs = ["temp_file_test.cpp"],
    deps = [
        ":temp_file",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "unicode",
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["unicode.cpp"],
        "//conditions:default": [],
    }),
    hdrs = ["unicode.h"],
)
