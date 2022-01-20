load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "trimmer",
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["trimmer_msvc.cpp"],
        "//conditions:default": [],
    }),
    deps = [
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/util:unicode",
        "//subtitler/util:duration_format",
        "//subtitler/util:temp_file",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
