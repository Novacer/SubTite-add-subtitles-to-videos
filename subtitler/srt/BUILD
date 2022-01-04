load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
  name = "subrip_item",
  hdrs = ["subrip_item.h"],
  srcs = ["subrip_item.cpp"],
  deps = [
    "//subtitler/util:duration_format",
    "@howard_hinnant_date//:date",
  ],
)

cc_test(
  name = "subrip_item_test",
  size = "small",
  srcs = ["subrip_item_test.cpp"],
  deps = [
    ":subrip_item",
    "@com_google_googletest//:gtest_main",
  ],
)

cc_library(
  name = "subrip_file",
  hdrs = ["subrip_file.h"],
  srcs = ["subrip_file.cpp"],
  deps = [
    ":subrip_item",
  ],
)

cc_test(
  name = "subrip_file_test",
  size = "small",
  srcs = ["subrip_file_test.cpp"],
  deps = [
    ":subrip_file",
    ":subrip_item",
    "@com_google_googletest//:gtest_main",
  ],
)
