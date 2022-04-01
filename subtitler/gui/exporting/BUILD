load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

# One target needs to contain hdrs and srcs for export dialog and tasks,
# since they have a circular dependency in the .cpp files.
qt_cc_library(
    name = "export_dialog",
    srcs = [
        "export_dialog.cpp",
        "tasks/burn_subtitle_task.cpp",
        "tasks/remux_subtitle_task.cpp",
    ],
    hdrs = [
        "export_dialog.h",
        "tasks/burn_subtitle_task.h",
        "tasks/remux_subtitle_task.h",
    ],
    deps = [
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/video/processing:ffmpeg",
        "//subtitler/video/processing:progress_parser",
        "//subtitler/video/util:video_utils",
        "@qt//:qt_widgets",
    ],
)
