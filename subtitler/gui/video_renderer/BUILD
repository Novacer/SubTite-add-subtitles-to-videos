load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "opengl_renderer",
    srcs = ["opengl_renderer.cpp"],
    hdrs = ["opengl_renderer.h"],
    deps = [
        "@qt//:qt_widgets",
        "@qt//:qt_multimedia",
    ],
)
