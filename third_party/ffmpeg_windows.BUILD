load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

FFMPEG_LIBRARIES = [
    # name, library (.lib) name, dll name, deps
    ("libavutil", "avutil", "avutil-56", []),
    ("libswscale", "swscale", "swscale-5", []),
    ("libswresample", "swresample", "swresample-3", []),
    ("libavcodec", "avcodec", "avcodec-58", []),
    ("libavformat", "avformat", "avformat-58", []),
    ("libavdevice", "avdevice", "avdevice-58", []),
    ("libavfilter", "avfilter", "avfilter-7", []),
]

[
    cc_import(
        name = "ffmpeg_%s_import" % name,
        hdrs = glob(["include/%s/**" % name]),
        interface_library = "lib/%s.lib" % library_name,
        shared_library = "bin/%s.dll" % windows_dll_name,
    )
    for name, library_name, windows_dll_name, _ in FFMPEG_LIBRARIES
]

[
    cc_library(
        name = "ffmpeg_%s" % name,
        hdrs = glob(["include/%s/**" % name]),
        includes = ["include"],
        strip_include_prefix = "include/",
        target_compatible_with = ["@platforms//os:windows"],
        visibility = ["//visibility:public"],
        deps = dependencies + [":ffmpeg_%s_import" % name],
    )
    for name, _, _, dependencies in FFMPEG_LIBRARIES
]
