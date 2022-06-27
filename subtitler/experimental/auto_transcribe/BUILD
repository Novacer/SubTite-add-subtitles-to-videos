load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "auto_transcribe",
    srcs = ["auto_transcribe_msvc.cpp"],
    target_compatible_with = [
        "@platforms//os:windows",
    ],
    deps = [
        "//subtitler/srt:subrip_file",
        "//subtitler/speech_recognition:auto_transcriber",
        "//subtitler/speech_recognition/cloud_service:microsoft_cognitive_service",
        "//subtitler/speech_recognition/cloud_service:cloud_service_base",
        "//subtitler/speech_recognition/languages:english_us",
        "//subtitler/speech_recognition/languages:language",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_nlohmann_json//:json",
    ] + select({
        "@bazel_tools//src/conditions:windows": ["@microsoft_cognitive_speech//:microsoft_cognitive_speech_cxx"],
        "//conditions:default": [],
    }),
)
