load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//subtitler:__subpackages__"])

cc_library(
    name = "cloud_service_base",
    hdrs = ["cloud_service_base.h"],
    deps = [
        "@com_github_nlohmann_json//:json",
    ],
)

cc_library(
    name = "microsoft_cognitive_service",
    srcs = ["microsoft_cognitive_service.cpp"],
    hdrs = ["microsoft_cognitive_service.h"],
    target_compatible_with = [
        "@platforms//os:windows",
    ],
    deps = [
        ":cloud_service_base",
        "@com_github_nlohmann_json//:json",
    ],
)

cc_library(
    name = "mock_microsoft_cognitive_service",
    testonly = True,
    hdrs = ["mock_microsoft_cognitive_service.h"],
    deps = [
        ":cloud_service_base",
        ":microsoft_cognitive_service",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest",
    ],
)
