load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "settings_window",
    srcs = ["settings_window.cpp"],
    hdrs = ["settings_window.h"],
    deps = [
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "main_window",
    srcs = ["main_window.cpp"],
    hdrs = ["main_window.h"],
    copts = select({
        "@platforms//os:windows": [
            "/DQT_BUILD_QTAVPLAYER_LIB",
            "/DQT_AVPLAYER_MULTIMEDIA",
        ],
        "//conditions:default": [
            "-DQT_BUILD_QTAVPLAYER_LIB",
            "-DQT_AVPLAYER_MULTIMEDIA",
        ],
    }),
    deps = [
        ":settings_window",
        "//subtitler/gui/auto_transcribe:auto_transcribe_window",
        "//subtitler/gui/auto_transcribe/login:login_msc",
        "//subtitler/gui/exporting:export_dialog",
        "//subtitler/gui/player_controls:play_button",
        "//subtitler/gui/player_controls:step_button",
        "//subtitler/gui/subtitle_editor:subtitle_editor",
        "//subtitler/gui/timeline:timer",
        "//subtitler/gui/timeline:timeline_window",
        "//subtitler/gui/video_renderer:opengl_renderer",
        "//subtitler/video/util:video_utils",
        "//subtitler/video/processing:downscaling",
        "@qt//:qt_widgets",
        "@qt//:qt_multimedia",
        "@qt//:qt_multimediawidgets",
    ] + select({
        "@platforms//os:windows": ["@qt_av_player_windows//:qt_av_player"],
        "//conditions:default": ["@qt_av_player_linux//:qt_av_player"],
    }) + select({
        "@platforms//os:windows": ["@ffmpeg_windows//:ffmpeg_libavformat"],
        "//conditions:default": ["@ffmpeg_linux//:ffmpeg_libavformat"],
    }),
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    copts = select({
        "@platforms//os:windows": [
            "/DQT_BUILD_QTAVPLAYER_LIB",
            "/DQT_AVPLAYER_MULTIMEDIA",
        ],
        "//conditions:default": [
            "-DQT_BUILD_QTAVPLAYER_LIB",
            "-DQT_AVPLAYER_MULTIMEDIA",
        ],
    }),
    deps = [
        ":main_window",
        "//subtitler/gui/resource:resources",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
        "@qt//:qt_network",
    ] + select({
        "@platforms//os:windows": ["@ffmpeg_windows//:ffmpeg_libavutil"],
        "//conditions:default": ["@ffmpeg_linux//:ffmpeg_libavutil"],
    }),
)
