load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_github_nlohmann_json",
    build_file = "//third_party:json.BUILD",
    sha256 = "1155fd1a83049767360e9a120c43c578145db3204d2b309eba49fbbedd0f4ed3",
    strip_prefix = "json-3.10.4",
    urls = ["https://github.com/nlohmann/json/archive/v3.10.4.tar.gz"],
)

http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
)

http_archive(
    name = "com_github_google_glog",
    sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
    strip_prefix = "glog-0.5.0",
    urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "5cf189eb6847b4f8fc603a3ffff3b0771c08eec7dd4bd961bfd45477dd13eb73",
    strip_prefix = "googletest-609281088cfefc76f9d0ce82e1ff6c30cc3591e5",
    # version 1.10.0
    urls = ["https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip"],
)

http_archive(
    name = "howard_hinnant_date",
    build_file = "//third_party:date.BUILD",
    sha256 = "7a390f200f0ccd207e8cff6757e04817c1a0aec3e327b006b7eb451c57ee3538",
    strip_prefix = "date-3.0.1",
    urls = ["https://github.com/HowardHinnant/date/archive/v3.0.1.tar.gz"],
)

# Configure QT Toolchains
http_archive(
    name = "com_justbuchanan_rules_qt",
    sha256 = "0ec127c6f60dc1900e915c420b990b9bec46dabdf9fa038adf5208284e1a4eb1",
    strip_prefix = "bazel_rules_qt-master",
    # Use custom fork which includes some fixes
    urls = ["https://github.com/Novacer/bazel_rules_qt/archive/refs/heads/master.zip"],
)

load("@com_justbuchanan_rules_qt//:qt_configure.bzl", "qt_configure")

qt_configure()

load("@local_config_qt//:local_qt.bzl", "local_qt_path")

new_local_repository(
    name = "qt",
    build_file = "@com_justbuchanan_rules_qt//:qt.BUILD",
    path = local_qt_path(),
)

load("@com_justbuchanan_rules_qt//tools:qt_toolchain.bzl", "register_qt_toolchains")

register_qt_toolchains()
# End Configure QT Toolchains

http_archive(
    name = "ffmpeg_windows",
    build_file = "//third_party:ffmpeg_windows.BUILD",
    strip_prefix = "ffmpeg-n4.4-latest-win64-lgpl-shared-4.4",
    urls = ["https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n4.4-latest-win64-lgpl-shared-4.4.zip"],
)

http_archive(
    name = "ffmpeg_linux",
    build_file = "//third_party:ffmpeg_linux.BUILD",
    strip_prefix = "ffmpeg-n4.4-latest-linux64-lgpl-shared-4.4",
    urls = ["https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n4.4-latest-linux64-lgpl-shared-4.4.tar.xz"],
)

# Need qt_av_player twice to work around genrule issue between Windows and Linux.
http_archive(
    name = "qt_av_player_windows",
    build_file = "//third_party:qt_av_player_windows.BUILD",
    sha256 = "255d8841dd271fafdc216562fb5b8c6c6713ad0aa6b1e0ca52abc5fe642d8157",
    strip_prefix = "QtAVPlayer-master",
    urls = ["https://github.com/Novacer/QtAVPlayer/archive/refs/heads/master.zip"],
)

http_archive(
    name = "qt_av_player_linux",
    build_file = "//third_party:qt_av_player_linux.BUILD",
    sha256 = "255d8841dd271fafdc216562fb5b8c6c6713ad0aa6b1e0ca52abc5fe642d8157",
    strip_prefix = "QtAVPlayer-master",
    urls = ["https://github.com/Novacer/QtAVPlayer/archive/refs/heads/master.zip"],
)
