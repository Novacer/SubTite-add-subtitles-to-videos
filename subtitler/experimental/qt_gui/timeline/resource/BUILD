load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_resource_file")

package(default_visibility = ["//subtitler/experimental:__subpackages__"])

# autogen_timeline.cpp is obtained from running rcc on the timeline.qrc file.
qt_resource_file(
    name = "resources",
    qrc_file = "timeline.qrc",
)
