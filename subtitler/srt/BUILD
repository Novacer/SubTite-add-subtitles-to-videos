load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "subrip_item",
    srcs = ["subrip_item.cpp"],
    hdrs = ["subrip_item.h"],
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
    srcs = ["subrip_file.cpp"],
    hdrs = ["subrip_file.h"],
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
        "//subtitler/util:temp_file",
        "@com_google_googletest//:gtest_main",
    ],
)
