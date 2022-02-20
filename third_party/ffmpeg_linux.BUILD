load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

FFMPEG_LIBRARIES = [
    # name, deps
    ("libavutil", []),
    ("libswscale", []),
    ("libswresample", []),
    ("libavcodec", []),
    ("libavformat", []),
    ("libavdevice", []),
    ("libavfilter", []),
]

[
    cc_import(
        name = "ffmpeg_%s_import" % name,
        hdrs = glob(["include/%s/**" % name]),
        shared_library = "lib/%s.so" % name,
    )
    for name, _ in FFMPEG_LIBRARIES
]

[
    cc_library(
        name = "ffmpeg_%s" % name,
        hdrs = glob(["include/%s/**" % name]),
        includes = ["include"],
        strip_include_prefix = "include/",
        target_compatible_with = ["@platforms//os:linux"],
        visibility = ["//visibility:public"],
        deps = dependencies + [":ffmpeg_%s_import" % name],
    )
    for name, dependencies in FFMPEG_LIBRARIES
]
