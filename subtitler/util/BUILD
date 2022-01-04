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
