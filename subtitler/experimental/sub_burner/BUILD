load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//subtitler/experimental:__subpackages__"])

cc_binary(
    name = "sub_burner",
    srcs = ["sub_burner.cpp"],
    deps = [
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/video/processing:ffmpeg",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
