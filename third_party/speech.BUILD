# Windows only Speech Recognition API
load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

cc_import(
    name = "microsoft_cognitive_speech_import",
    hdrs = glob(["build/native/include/**/*.h"]),
    interface_library = "build/native/x64/Release/Microsoft.CognitiveServices.Speech.core.lib",
    shared_library = "runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.core.dll",
)

cc_library(
    name = "microsoft_cognitive_speech_c",
    hdrs = glob(["build/native/include/c_api/*.h"]),
    includes = ["build/native/include"],
    strip_include_prefix = "build/native/include/c_api",
    target_compatible_with = ["@platforms//os:windows"],
    visibility = ["//visibility:public"],
    deps = [":microsoft_cognitive_speech_import"],
)

cc_library(
    name = "microsoft_cognitive_speech_cxx",
    hdrs = glob(["build/native/include/cxx_api/*.h"]),
    includes = ["build/native/include"],
    strip_include_prefix = "build/native/include/cxx_api",
    target_compatible_with = ["@platforms//os:windows"],
    visibility = ["//visibility:public"],
    deps = [
        ":microsoft_cognitive_speech_c",
        ":microsoft_cognitive_speech_import",
    ],
)
