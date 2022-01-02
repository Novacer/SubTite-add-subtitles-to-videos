load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
  name = "cli",
  srcs = ["main.cpp"],
  deps = [
    "//subtitler/play_video:ffplay",
    "//subtitler/subprocess:subprocess_executor",
    "@com_github_nlohmann_json//:json",
    "@com_github_gflags_gflags//:gflags",
    "@com_github_google_glog//:glog",
    "@howard_hinnant_date//:date",
  ],
)
