load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "play_button",
    srcs = ["play_button.cpp"],
    hdrs = ["play_button.h"],
    deps = ["@qt//:qt_widgets"],
)

qt_cc_library(
    name = "main_window",
    srcs = ["main_window.cpp"],
    hdrs = ["main_window.h"],
    copts = select({
        "@bazel_tools//src/conditions:windows": ["/DQT_BUILD_QTAVPLAYER_LIB"],
        "@bazel_tools//src/conditions:darwin": ["-DQT_BUILD_QTAVPLAYER_LIB"],
        "//conditions:default": ["-DQT_BUILD_QTAVPLAYER_LIB"],
    }),
    deps = [
        ":play_button",
        ":subtitle_editor",
        "//subtitler/gui/timeline:timer",
        "//subtitler/gui/timeline:timeline_window",
        "@qt//:qt_widgets",
        "@qt//:qt_multimedia",
        "@qt//:qt_multimediawidgets",
    ] + select({
        "@bazel_tools//src/conditions:windows": ["@qt_av_player_windows//:qt_av_player"],
        "@bazel_tools//src/conditions:darwin": ["@qt_av_player_linux//:qt_av_player"],
        "//conditions:default": ["@qt_av_player_linux//:qt_av_player"],
    }) + select({
        "@bazel_tools//src/conditions:windows": ["@ffmpeg_windows//:ffmpeg_libavformat"],
        "@bazel_tools//src/conditions:darwin": ["@ffmpeg_linux//:ffmpeg_libavformat"],
        "//conditions:default": ["@ffmpeg_linux//:ffmpeg_libavformat"],
    }),
)

qt_cc_library(
    name = "subtitle_editor",
    srcs = ["subtitle_editor.cpp"],
    hdrs = ["subtitle_editor.h"],
    deps = [
        "//subtitler/gui/timeline:subtitle_interval",
        "//subtitler/util:duration_format",
        "@qt//:qt_widgets",
    ],
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
        ":main_window",
        "//subtitler/gui/resource:resources",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ] + select({
        "@bazel_tools//src/conditions:windows": ["@ffmpeg_windows//:ffmpeg_libavutil"],
        "@bazel_tools//src/conditions:darwin": ["@ffmpeg_linux//:ffmpeg_libavutil"],
        "//conditions:default": ["@ffmpeg_linux//:ffmpeg_libavutil"],
    }),
)
