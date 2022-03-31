load("@rules_cc//cc:defs.bzl", "cc_library")

FFMPEG_LIBRARIES = [
    # name, soname (libavutil.so.56 etc), deps
    ("libavutil", "libavutil.so.56", []),
    ("libswscale", "libswscale.so.5", []),
    ("libswresample", "libswresample.so.3", []),
    ("libavcodec", "libavcodec.so.58", []),
    ("libavformat", "libavformat.so.58", []),
    ("libavdevice", "libavdevice.so.58", []),
    ("libavfilter", "libavfilter.so.7", []),
]

[
    cc_import(
        name = "ffmpeg_%s_import" % name,
        hdrs = glob(["include/%s/**" % name]),
        shared_library = "lib/%s" % soname,
    )
    for name, soname, _ in FFMPEG_LIBRARIES
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
    for name, _, dependencies in FFMPEG_LIBRARIES
]
