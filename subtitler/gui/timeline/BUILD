load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "indicator",
    srcs = ["indicator.cpp"],
    hdrs = ["indicator.h"],
    deps = [
        "//subtitler/gui/resource:resources",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "ruler",
    srcs = ["ruler.cpp"],
    hdrs = ["ruler.h"],
    deps = [
        ":indicator",
        ":subtitle_interval",
        "//subtitler/gui/resource:resources",
        "//subtitler/util:duration_format",
        "@qt//:qt_core",
        "@qt//:qt_gui",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "timer",
    srcs = ["timer.cpp"],
    hdrs = ["timer.h"],
    deps = [
        "//subtitler/util:duration_format",
        "@qt//:qt_core",
        "@qt//:qt_gui",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "zoomer",
    srcs = ["zoomer.cpp"],
    hdrs = ["zoomer.h"],
    deps = [
        ":indicator",
        "//subtitler/gui/resource:resources",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "timeline_window",
    srcs = ["timeline.cpp"],
    hdrs = ["timeline.h"],
    deps = [
        ":ruler",
        ":zoomer",
        "//subtitler/gui/resource:resources",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "subtitle_interval",
    srcs = ["subtitle_interval.cpp"],
    hdrs = ["subtitle_interval.h"],
    deps = [
        "//subtitler/gui/resource:resources",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ],
)
