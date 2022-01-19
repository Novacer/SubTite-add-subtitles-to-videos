load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "ffplay",
    srcs = ["ffplay.cpp"],
    hdrs = ["ffplay.h"],
    deps = [
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/util:font_config",
        "@howard_hinnant_date//:date",
    ],
)

cc_test(
    name = "ffplay_test",
    size = "small",
    srcs = ["ffplay_test.cpp"],
    deps = [
        ":ffplay",
        "//subtitler/subprocess:mock_subprocess_executor",
        "//subtitler/util:font_config",
        "@com_google_googletest//:gtest_main",
    ],
)
