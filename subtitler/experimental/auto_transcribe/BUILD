load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "auto_transcribe",
    srcs = ["auto_transcribe_msvc.cpp"],
    target_compatible_with = [
        "@platforms//os:windows",
    ],
    deps = [
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_nlohmann_json//:json",
        "@microsoft_cognitive_speech//:microsoft_cognitive_speech_cxx",
    ],
)
