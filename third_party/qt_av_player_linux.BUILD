load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

LINUX_EXCLUDE_LIST = [
    "**/qavhwdevice_vaapi_drm_egl.*",
    # "**/qavhwdevice_vaapi_x11_glx.*",
    "**/qavhwdevice_d3d11.*",
    "**/qavhwdevice_mediacodec.*",
    "**/qavhwdevice_videotoolbox.*",
]

qt_cc_library(
    name = "qt_av_player",
    srcs = glob(
        exclude = LINUX_EXCLUDE_LIST,
        include = ["src/QtAVPlayer/*.cpp"],
    ),
    hdrs = glob(
        exclude = LINUX_EXCLUDE_LIST,
        include = ["src/QtAVPlayer/*.h"],
    ),
    # includes = ["src/QtAvPlayer"],
    strip_include_prefix = "src/",
    target_compatible_with = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
    deps = [
        "@ffmpeg_linux//:ffmpeg_libavcodec",
        "@ffmpeg_linux//:ffmpeg_libavdevice",
        "@ffmpeg_linux//:ffmpeg_libavfilter",
        "@ffmpeg_linux//:ffmpeg_libavformat",
        "@ffmpeg_linux//:ffmpeg_libavutil",
        "@ffmpeg_linux//:ffmpeg_libswresample",
        "@ffmpeg_linux//:ffmpeg_libswscale",
        "@qt//:qt_core",
        "@qt//:qt_gui",
        "@qt//:qt_multimedia",
    ],
)
