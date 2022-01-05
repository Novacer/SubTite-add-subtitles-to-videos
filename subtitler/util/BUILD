load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
  name = "duration_format",
  hdrs = ["duration_format.h"],
  srcs = ["duration_format.cpp"],
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
  hdrs = ["font_config.h"],
  srcs = select({
    "@bazel_tools//src/conditions:windows": ["font_config_msvc.cpp"],
    "@bazel_tools//src/conditions:darwin": ["font_config_gcc.cpp"],
     "//conditions:default": ["font_config_gcc.cpp"],
  }),
  deps = [],
)

cc_library(
  name = "temp_file",
  hdrs = ["temp_file.h"],
  srcs = ["temp_file.cpp"],
  deps = [],
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
