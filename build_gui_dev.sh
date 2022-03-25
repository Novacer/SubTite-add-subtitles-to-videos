#!/bin/bash

# Meant for building the SubTite GUI for development purposes.
# If you want the production build, see deploy/deploy.sh instead!

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

BAZEL_BUILD_DIR=bazel-bin/subtitler/gui

set -e

if [[ ${machine} == "MinGw" || ${machine} == "CYGWIN" ]]; then
    echo "Using OS: Windows"
    if [[ -z ${QT5_INSTALL_PATH} ]]; then
        echo "Please set QT5_INSTALL_PATH env variable to be the QT5 path containing lib, bin, plugins etc."
        exit 1
    else
        echo "Using QT5 Path: ${QT5_INSTALL_PATH}"
    fi

    bazel build --config=vs2019 //subtitler/gui:main
    # Copy audio plugins to bazel-bin
    if [ ! -d ${BAZEL_BUILD_DIR}/plugins ]; then
        mkdir -p ${BAZEL_BUILD_DIR}/plugins
        cp -r ${QT5_INSTALL_PATH}/plugins/audio ${BAZEL_BUILD_DIR}/plugins/audio
        cp -r ${QT5_INSTALL_PATH}/plugins/platforms ${BAZEL_BUILD_DIR}/plugins/platforms
    fi
    # Copy ffmpeg binary to bazel-bin
    if [ ! -f ${BAZEL_BUILD_DIR}/ffmpeg.exe ]; then
        cp bazel-subtitler/external/ffmpeg_windows/bin/ffmpeg.exe ${BAZEL_BUILD_DIR}/ffmpeg.exe
    fi

elif [[ ${machine} == "Linux" ]]; then
    echo "Using OS: Linux"
    bazel build --config=gcc-asan //subtitler/gui:main

    # Copy ffmpeg binary to bazel-bin
    if [ ! -f ${BAZEL_BUILD_DIR}/ffmpeg ]; then
        cp bazel-subtitler/external/ffmpeg_linux/bin/ffmpeg.exe ${BAZEL_BUILD_DIR}/ffmpeg
    fi

else
    echo "Unsupported OS: ${machine}"
    exit 1
fi
