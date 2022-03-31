load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "export_dialog",
    srcs = ["export_dialog.cpp"],
    hdrs = ["export_dialog.h"],
    deps = [
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/video/processing:ffmpeg",
        "//subtitler/video/processing:progress_parser",
        "//subtitler/video/util:video_utils",
        "@qt//:qt_widgets",
    ],
)
