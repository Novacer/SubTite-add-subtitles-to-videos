load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/experimental:__subpackages__"])

qt_cc_library(
    name = "play_button",
    srcs = ["play_button.cpp"],
    hdrs = ["play_button.h"],
    deps = ["@qt//:qt_widgets"],
)

qt_cc_library(
    name = "player_window",
    srcs = ["player_window.cpp"],
    hdrs = ["player_window.h"],
    copts = select({
        "@bazel_tools//src/conditions:windows": ["/DQT_BUILD_QTAVPLAYER_LIB"],
        "@bazel_tools//src/conditions:darwin": ["-DQT_BUILD_QTAVPLAYER_LIB"],
        "//conditions:default": ["-DQT_BUILD_QTAVPLAYER_LIB"],
    }),
    deps = [
        ":play_button",
        "//subtitler/experimental/qt_gui/timeline:timer",
        "//subtitler/experimental/qt_gui/timeline:timeline_window",
        "@qt//:qt_widgets",
        "@qt//:qt_multimedia",
        "@qt//:qt_multimediawidgets",
    ] + select({
        "@bazel_tools//src/conditions:windows": ["@qt_av_player_windows//:qt_av_player"],
        "@bazel_tools//src/conditions:darwin": ["@qt_av_player_linux//:qt_av_player"],
        "//conditions:default": ["@qt_av_player_linux//:qt_av_player"],
    }),
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    copts = select({
        "@bazel_tools//src/conditions:windows": ["/DQT_BUILD_QTAVPLAYER_LIB"],
        "@bazel_tools//src/conditions:darwin": ["-DQT_BUILD_QTAVPLAYER_LIB"],
        "//conditions:default": ["-DQT_BUILD_QTAVPLAYER_LIB"],
    }),
    deps = [
        ":player_window",
        "//subtitler/experimental/qt_gui/timeline/resource:resources",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ],
)
