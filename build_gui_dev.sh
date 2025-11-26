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
    if [[ -z ${MCS_INSTALL_LOCATION} ]]; then
        echo "Please set MCS_INSTALL_LOCATION env variable to be the MicrosoftCognitiveSpeech path containing lib, runtime folders etc."
        exit 1
    else
        echo "Using MicrosoftCognitiveSpeech package at: ${MCS_INSTALL_LOCATION}"
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
        cp bazel-$(basename $(pwd))/external/ffmpeg_windows/bin/ffmpeg.exe ${BAZEL_BUILD_DIR}/ffmpeg.exe
    fi
    # Copy MicrosoftCognitiveServices runtime extensions
    cp "${MCS_INSTALL_LOCATION}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.extension".* ${BAZEL_BUILD_DIR}

elif [[ ${machine} == "Linux" ]]; then
    echo "Using OS: Linux"
    bazel build --config=gcc-asan //subtitler/gui:main

    # Copy ffmpeg binary to bazel-bin
    if [ ! -f ${BAZEL_BUILD_DIR}/ffmpeg ]; then
        cp bazel-$(basename $(pwd))/external/ffmpeg_linux/bin/ffmpeg ${BAZEL_BUILD_DIR}/ffmpeg
    fi

else
    echo "Unsupported OS: ${machine}"
    exit 1
fi
