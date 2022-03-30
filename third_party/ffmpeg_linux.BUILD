load("@rules_cc//cc:defs.bzl", "cc_library")

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
    cc_library(
        name = "ffmpeg_%s" % name,
        srcs = ["lib/%s.so" % name],
        hdrs = glob(["include/%s/**" % name]),
        includes = ["include"],
        strip_include_prefix = "include/",
        target_compatible_with = ["@platforms//os:linux"],
        visibility = ["//visibility:public"],
        deps = dependencies,
    )
    for name, dependencies in FFMPEG_LIBRARIES
]
