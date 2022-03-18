load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "subtitle_editor",
    srcs = ["subtitle_editor.cpp"],
    hdrs = ["subtitle_editor.h"],
    deps = [
        ":position_buttons",
        "//subtitler/gui/timeline:subtitle_interval",
        "//subtitler/util:duration_format",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "position_buttons",
    srcs = ["position_buttons.cpp"],
    hdrs = ["position_buttons.h"],
    deps = [
        "@qt//:qt_widgets",
    ],
)
