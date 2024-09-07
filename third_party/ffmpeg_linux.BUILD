load("@rules_cc//cc:defs.bzl", "cc_library")

# Bazel appears to traverse at most one symlink level (I'm not 100% sure).
# We have libavutil.so => libavutil.so.56 => libavutil.so.56.xxx
# In any case, using libavutil.so in cc_import means the program can compile
# but when running it will not find the shared lib (one pre-installed on the
# system may be used instead).
# If cc_import instead gets libavutil.so.56 then bazel uses the correct lib.
FFMPEG_LIBRARIES = [
    # name, soname (libavutil.so.56 etc), deps
    ("libavutil", "libavutil.so.58", []),
    ("libswscale", "libswscale.so.7", []),
    ("libswresample", "libswresample.so.4", []),
    ("libavcodec", "libavcodec.so.60", []),
    ("libavformat", "libavformat.so.60", []),
    ("libavdevice", "libavdevice.so.60", []),
    ("libavfilter", "libavfilter.so.9", []),
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
