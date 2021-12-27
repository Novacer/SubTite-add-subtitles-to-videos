load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler/cli:__subpackages__"])

cc_library(
  name = "subprocess_executor",
  hdrs = ["subprocess_executor.h"],
  srcs = select({
    "@bazel_tools//src/conditions:windows": ["subprocess_executor_msvc.cpp"],
    "@bazel_tools//src/conditions:darwin": ["subprocess_executor_gcc.cpp"],
     "//conditions:default": ["subprocess_executor_gcc.cpp"],
  }),
  deps = [],
)

cc_test(
  name = "subprocess_executor_test",
  size = "small",
  srcs = select({
    "@bazel_tools//src/conditions:windows": ["subprocess_executor_msvc_test.cpp"],
    "@bazel_tools//src/conditions:darwin": ["subprocess_executor_gcc_test.cpp"],
     "//conditions:default": ["subprocess_executor_gcc_test.cpp"],
  }),
  deps = [
    ":subprocess_executor",
    "@com_google_googletest//:gtest_main",
  ],
)
