load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

PLATFORM_SPECIFIC_WILDCARDS = [
    "**/qavhwdevice_vaapi_drm_egl.*",
    "**/qavhwdevice_vaapi_x11_glx.*",
    # We use this one on windows so we do not exclude it.
    # "**/qavhwdevice_d3d11.*",
    "**/qavhwdevice_mediacodec.*",
    "**/qavhwdevice_videotoolbox.*",
]

qt_cc_library(
    name = "qt_av_player",
    srcs = glob(
        exclude = PLATFORM_SPECIFIC_WILDCARDS,
        include = ["src/QtAvPlayer/*.cpp"],
    ),
    hdrs = glob(
        exclude = PLATFORM_SPECIFIC_WILDCARDS,
        include = ["src/QtAvPlayer/*.h"],
    ),
    # includes = ["src/QtAvPlayer"],
    strip_include_prefix = "src/",
    target_compatible_with = ["@platforms//os:windows"],
    visibility = ["//visibility:public"],
    deps = [
        "@ffmpeg_windows//:ffmpeg_libavcodec",
        "@ffmpeg_windows//:ffmpeg_libavdevice",
        "@ffmpeg_windows//:ffmpeg_libavfilter",
        "@ffmpeg_windows//:ffmpeg_libavformat",
        "@ffmpeg_windows//:ffmpeg_libavutil",
        "@ffmpeg_windows//:ffmpeg_libswresample",
        "@ffmpeg_windows//:ffmpeg_libswscale",
        "@qt//:qt_core",
        "@qt//:qt_gui",
        "@qt//:qt_multimedia",
    ],
)
