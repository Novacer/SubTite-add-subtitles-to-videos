load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
  name = "cli",
  srcs = ["main.cpp"],
  deps = [
    "@com_github_nlohmann_json//:json",
    "@com_github_gflags_gflags//:gflags",
    "@com_github_google_glog//:glog",
  ],
)

# Example of how to select platform specific implementations.
# https://stackoverflow.com/questions/48860910
# TODO remove once used.
# cc_library(
#   name = "impl",
#   srcs = ["Implementation.cpp"] + select({
#     "@bazel_tools//src/conditions:windows": ["ImplementationWin.cpp"],
#     "@bazel_tools//src/conditions:darwin": ["ImplementationMacOS.cpp"],
#      "//conditions:default": ["ImplementationLinux.cpp"],
#   }),
#   # .. same for hdrs and data
# )
