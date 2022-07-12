# Windows Only libsodium pre-compiled binaries.
load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

cc_import(
    name = "libsodium_import",
    hdrs = ["include/sodium.h"] + glob(["include/sodium/*.h"]),
    interface_library = "x64/Release/v142/dynamic/libsodium.lib",
    shared_library = "x64/Release/v142/dynamic/libsodium.dll",
    static_library = "x64/Release/v142/static/libsodium.lib",
)

cc_library(
    name = "libsodium",
    hdrs = ["include/sodium.h"],
    includes = ["include/"],
    linkstatic = True,
    strip_include_prefix = "include/",
    target_compatible_with = ["@platforms//os:windows"],
    visibility = ["//visibility:public"],
    deps = [":libsodium_import"],
)
