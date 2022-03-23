load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "subprocess_executor",
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["subprocess_executor_msvc.cpp"],
        "@bazel_tools//src/conditions:linux": ["subprocess_executor_gcc.cpp"],
        "//conditions:default": ["subprocess_executor_gcc.cpp"],
    }),
    hdrs = ["subprocess_executor.h"],
    deps = [
        "//subtitler/util:unicode",
    ],
)

cc_test(
    name = "subprocess_executor_test",
    timeout = "short",
    srcs = select({
        "@bazel_tools//src/conditions:windows": ["subprocess_executor_msvc_test.cpp"],
        "@bazel_tools//src/conditions:linux": ["subprocess_executor_gcc_test.cpp"],
        "//conditions:default": ["subprocess_executor_gcc_test.cpp"],
    }),
    deps = [
        ":subprocess_executor",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "mock_subprocess_executor",
    testonly = True,
    hdrs = ["mock_subprocess_executor.h"],
    deps = ["@com_google_googletest//:gtest"],
)
