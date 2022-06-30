load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

# One target needs to contain hdrs and srcs for export dialog and tasks,
# since they have a circular dependency in the .cpp files.
qt_cc_library(
    name = "auto_transcribe_window",
    srcs = [
        "auto_transcribe_window.cpp",
    ],
    hdrs = [
        "auto_transcribe_window.h",
    ],
    deps = [
        "@qt//:qt_widgets",
    ],
)
