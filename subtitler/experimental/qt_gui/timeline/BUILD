load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

qt_cc_library(
    name = "indicator",
    srcs = ["indicator.cpp"],
    hdrs = ["indicator.h"],
    deps = [
        "//subtitler/experimental/qt_gui/timeline/resource:resources",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "keyline",
    srcs = ["keyline.cpp"],
    hdrs = ["keyline.h"],
    deps = [
        "@qt//:qt_core",
        "@qt//:qt_gui",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "keypoint",
    srcs = ["keypoint.cpp"],
    hdrs = ["keypoint.h"],
    deps = [
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "ruler",
    srcs = ["ruler.cpp"],
    hdrs = ["ruler.h"],
    deps = [
        ":indicator",
        "//subtitler/experimental/qt_gui/timeline/resource:resources",
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
        "//subtitler/experimental/qt_gui/timeline/resource:resources",
        "@qt//:qt_widgets",
    ],
)

qt_cc_library(
    name = "mainwindow",
    srcs = ["mainwindow.cpp"],
    hdrs = ["mainwindow.h"],
    deps = [
        ":ruler",
        ":zoomer",
        "//subtitler/experimental/qt_gui/timeline/resource:resources",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ],
)

cc_binary(
    name = "timeline",
    srcs = ["main.cpp"],
    deps = [
        ":mainwindow",
        "//subtitler/experimental/qt_gui/timeline/resource:resources",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ],
)
