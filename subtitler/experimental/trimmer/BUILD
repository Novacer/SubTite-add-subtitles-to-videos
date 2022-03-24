load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "trimmer",
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["trimmer_msvc.cpp"],
        "//conditions:default": ["trimmer_gcc.cpp"],
    }),
    deps = [
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/util:duration_format",
        "//subtitler/util:temp_file",
        "//subtitler/util:unicode",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)
