load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "auto_transcribe",
    srcs = ["auto_transcribe_msvc.cpp"],
    target_compatible_with = [
        "@platforms//os:windows",
    ],
    deps = [
        "//subtitler/speech_recognition:auto_transcriber",
        "//subtitler/speech_recognition/cloud_service:cloud_service_base",
        "//subtitler/speech_recognition/cloud_service:microsoft_cognitive_service",
        "//subtitler/speech_recognition/languages:english_us",
        "//subtitler/speech_recognition/languages:language",
        "//subtitler/srt:subrip_file",
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/util:temp_file",
        "//subtitler/util:unicode",
        "//subtitler/video/processing:ffmpeg",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
        "@com_github_nlohmann_json//:json",
    ] + select({
        "@platforms//os:windows": ["@microsoft_cognitive_speech//:microsoft_cognitive_speech_cxx"],
        "//conditions:default": [],
    }),
)
