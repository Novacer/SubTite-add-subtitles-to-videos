load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

qt_cc_library(
    name = "main_window",
    srcs = ["main_window.cpp"],
    hdrs = ["main_window.h"],
    deps = ["@qt//:qt_widgets"],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [
        ":main_window",
        "@qt//:qt_widgets",
        "@qt//:qt_multimedia",
        "@qt//:qt_multimediawidgets",
    ] + select({
        "@bazel_tools//src/conditions:windows": ["@qt_av_player_windows//:qt_av_player"],
        "@bazel_tools//src/conditions:darwin": ["@qt_av_player_linux//:qt_av_player"],
        "//conditions:default": ["@qt_av_player_linux//:qt_av_player"],
    }),
)
