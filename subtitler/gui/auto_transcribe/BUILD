load("@com_justbuchanan_rules_qt//:qt.bzl", "qt_cc_library")

package(default_visibility = ["//subtitler/gui:__subpackages__"])

qt_cc_library(
    name = "auto_transcribe_window",
    srcs = select({
        "@bazel_tools//src/conditions:windows": [
            "auto_transcribe_window_msvc.cpp",
            "tasks/transcribe_task.cpp",
        ],
        "//conditions:default": ["auto_transcribe_window_gcc.cpp"],
    }),
    hdrs = [
        "auto_transcribe_window.h",
        "tasks/transcribe_task.h",
    ],
    deps = [
        "//subtitler/speech_recognition:auto_transcriber",
        "//subtitler/srt:subrip_file",
        "//subtitler/subprocess:subprocess_executor",
        "//subtitler/video/processing:ffmpeg",
        "@com_github_nlohmann_json//:json",
        "@qt//:qt_widgets",
    ] + select({
        "@bazel_tools//src/conditions:windows": [
            "//subtitler/speech_recognition/cloud_service:microsoft_cognitive_service",
            "//subtitler/speech_recognition/languages:english_us",
        ],
        "//conditions:default": [],
    }),
)
