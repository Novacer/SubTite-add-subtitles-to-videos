load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
  name = "cli",
  srcs = select({
    "@bazel_tools//src/conditions:windows": ["main_msvc.cpp"],
    "@bazel_tools//src/conditions:darwin": ["main_gcc.cpp"],
     "//conditions:default": ["main_gcc.cpp"],
  }),
  deps = [
    ":commands",
    "//subtitler/play_video:ffplay",
    "//subtitler/subprocess:subprocess_executor",
    "@com_github_nlohmann_json//:json",
    "@com_github_gflags_gflags//:gflags",
    "@com_github_google_glog//:glog",
  ],
)

cc_library(
  name = "commands",
  hdrs = ["commands.h"],
  srcs = ["commands.cpp"],
  deps = [
    "//subtitler/play_video:ffplay",
    "//subtitler/srt:subrip_file",
    "//subtitler/srt:subrip_item",
    "//subtitler/util:duration_format",
    "//subtitler/util:temp_file",
    "@howard_hinnant_date//:date",
  ],
)

cc_test(
  name = "commands_test",
  size = "medium",
  srcs = ["commands_test.cpp"],
  deps = [
    ":commands",
    "//subtitler/subprocess:mock_subprocess_executor",
    "//subtitler/play_video:ffplay",
    "@com_google_googletest//:gtest_main",
  ],
  # Run serially since there are file system dependencies.
  tags = ["exclusive"],
)
