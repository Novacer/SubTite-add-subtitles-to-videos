load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "file_encryption",
    srcs = ["file_encryption.cpp"],
    deps = select({
        "@bazel_tools//src/conditions:windows": ["@libsodium_windows//:libsodium"],
        "//conditions:default": [],
    }),
    linkopts = select({
        "@bazel_tools//src/conditions:windows": [""],
        "//conditions:default": ["-lsodium"],
    })
)
