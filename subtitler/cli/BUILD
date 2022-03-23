load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "cli",
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["main_msvc.cpp"],
        "@bazel_tools//src/conditions:linux": ["main_gcc.cpp"],
        "//conditions:default": ["main_gcc.cpp"],
    }),
    deps = [
        ":commands",
        "//subtitler/cli/io:input",
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/util:unicode",
        "//subtitler/video/metadata:ffprobe",
        "//subtitler/video/player:ffplay",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "commands",
    srcs = ["commands.cpp"],
    hdrs = ["commands.h"],
    deps = [
        "//subtitler/cli/io:input",
        "//subtitler/srt:subrip_file",
        "//subtitler/srt:subrip_item",
        "//subtitler/util:duration_format",
        "//subtitler/util:temp_file",
        "//subtitler/video/metadata:ffprobe",
        "//subtitler/video/player:ffplay",
        "@howard_hinnant_date//:date",
    ],
)

cc_test(
    name = "commands_test",
    size = "small",
    srcs = ["commands_test.cpp"],
    # Run serially since there are file system dependencies.
    tags = ["exclusive"],
    deps = [
        ":commands",
        "//subtitler/subprocess:mock_subprocess_executor",
        "//subtitler/video/metadata:ffprobe",
        "//subtitler/video/player:ffplay",
        "@com_google_googletest//:gtest_main",
    ],
)
