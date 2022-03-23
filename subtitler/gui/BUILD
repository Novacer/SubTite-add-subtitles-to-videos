load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "main_window",
    srcs = ["main_window.cpp"],
    hdrs = ["main_window.h"],
    copts = select({
        "@bazel_tools//src/conditions:windows": ["/DQT_BUILD_QTAVPLAYER_LIB"],
        "@bazel_tools//src/conditions:linux": ["-DQT_BUILD_QTAVPLAYER_LIB"],
        "//conditions:default": ["-DQT_BUILD_QTAVPLAYER_LIB"],
    }),
    deps = [
        "//subtitler/gui/player_controls:play_button",
        "//subtitler/gui/player_controls:step_button",
        "//subtitler/gui/subtitle_editor:subtitle_editor",
        "//subtitler/gui/timeline:timer",
        "//subtitler/gui/timeline:timeline_window",
        "@qt//:qt_widgets",
        "@qt//:qt_multimedia",
        "@qt//:qt_multimediawidgets",
    ] + select({
        "@bazel_tools//src/conditions:windows": ["@qt_av_player_windows//:qt_av_player"],
        "@bazel_tools//src/conditions:linux": ["@qt_av_player_linux//:qt_av_player"],
        "//conditions:default": ["@qt_av_player_linux//:qt_av_player"],
    }) + select({
        "@bazel_tools//src/conditions:windows": ["@ffmpeg_windows//:ffmpeg_libavformat"],
        "@bazel_tools//src/conditions:linux": ["@ffmpeg_linux//:ffmpeg_libavformat"],
        "//conditions:default": ["@ffmpeg_linux//:ffmpeg_libavformat"],
    }),
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    copts = select({
        "@bazel_tools//src/conditions:windows": ["/DQT_BUILD_QTAVPLAYER_LIB"],
        "@bazel_tools//src/conditions:linux": ["-DQT_BUILD_QTAVPLAYER_LIB"],
        "//conditions:default": ["-DQT_BUILD_QTAVPLAYER_LIB"],
    }),
    deps = [
        ":main_window",
        "//subtitler/gui/resource:resources",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ] + select({
        "@bazel_tools//src/conditions:windows": ["@ffmpeg_windows//:ffmpeg_libavutil"],
        "@bazel_tools//src/conditions:linux": ["@ffmpeg_linux//:ffmpeg_libavutil"],
        "//conditions:default": ["@ffmpeg_linux//:ffmpeg_libavutil"],
    }),
)
