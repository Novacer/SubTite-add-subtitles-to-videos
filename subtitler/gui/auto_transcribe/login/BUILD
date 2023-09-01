load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "login_msc",
    srcs = select({
        "@platforms//os:windows": ["login_msc_msvc.cpp"],
        "//conditions:default": ["login_msc_gcc.cpp"],
    }),
    hdrs = [
        "login_msc.h",
    ],
    deps = [
        "//subtitler/encryption:file_encryption",
        "@com_github_nlohmann_json//:json",
        "@qt//:qt_core",
        "@qt//:qt_widgets",
    ],
)
