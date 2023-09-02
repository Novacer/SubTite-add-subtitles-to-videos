load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

WINDOWS_EXCLUDE_LIST = [
    "**/qavhwdevice_vaapi_drm_egl*",
    "**/qavhwdevice_vaapi_x11_glx*",
    # "**/qavhwdevice_d3d11*",
    # "**/qavhwdevice_dxva2*",
    "**/qavhwdevice_mediacodec*",
    "**/qavhwdevice_videotoolbox*",
    "**/qavhwdevice_vdpau*",
    "**/qavandroidsurfacetexture*",
]

qt_cc_library(
    name = "qt_av_player",
    srcs = glob(
        include = ["src/QtAVPlayer/*.cpp"],
        exclude = WINDOWS_EXCLUDE_LIST,
    ),
    hdrs = glob(
        include = ["src/QtAVPlayer/*.h"],
        exclude = WINDOWS_EXCLUDE_LIST,
    ),
    copts = [
        "/DQT_BUILD_QTAVPLAYER_LIB",
        "/DQT_AVPLAYER_MULTIMEDIA",
    ],
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
        "@qt//:qt_concurrent",
        "@qt//:qt_core",
        "@qt//:qt_gui",
        "@qt//:qt_multimedia",
    ],
)
