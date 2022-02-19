load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

qt_cc_library(
    name = "main_window",
    srcs = ["main_window.cpp"],
    hdrs = ["main_window.h"],
    deps = ["@qt//:qt_widgets"],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [
        ":main_window",
        "@qt//:qt_widgets",
    ],
)
