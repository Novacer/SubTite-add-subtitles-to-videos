load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

LINUX_EXCLUDE_LIST = [
    "**/qavhwdevice_vaapi_drm_egl*",
    # "**/qavhwdevice_vaapi_x11_glx*",
    "**/qavhwdevice_d3d11*",
    "**/qavhwdevice_dxva2*",
    "**/qavhwdevice_mediacodec*",
    "**/qavhwdevice_videotoolbox*",
    "**/qavhwdevice_vdpau*",
    "**/qavandroidsurfacetexture*",
]

qt_cc_library(
    name = "qt_av_player",
    srcs = glob(
        include = ["src/QtAVPlayer/*.cpp"],
        exclude = LINUX_EXCLUDE_LIST,
    ),
    hdrs = glob(
        include = ["src/QtAVPlayer/*.h"],
        exclude = LINUX_EXCLUDE_LIST,
    ),
    copts = [
        "-DQT_BUILD_QTAVPLAYER_LIB",
        "-DQT_AVPLAYER_MULTIMEDIA",
    ],
    linkopts = [
        "-lGL",
        "-lX11",
        "-lva",
        "-lva-x11",
    ],
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
        "@qt//:qt_concurrent",
        "@qt//:qt_core",
        "@qt//:qt_gui",
        "@qt//:qt_multimedia",
    ],
)
