#!/bin/bash

if [[ -z ${QT5_INSTALL_PATH} ]]; then
    echo "Please set QT5_INSTALL_PATH env variable to be the QT5 install path containing lib, bin, plugins etc."
    exit 1
else
    echo "Using QT5 Path: ${QT5_INSTALL_PATH}"
fi

if [[ -z ${SUBTITE_RELEASE_PATH} ]]; then
    echo "Please set SUBTITE_RELEASE_PATH env variable to be the directory to copy release binary"
    exit 1
else
    echo "Using QT5 Path: ${SUBTITE_RELEASE_PATH}"
fi

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

# Instead of directly using the path, use a folder within the path so when
# we do the delete step we aren't wiping any important files by accident.
SUBTITE_RELEASE_PATH=${SUBTITE_RELEASE_PATH}/subtite-release

set -e

if [[ ${machine} == "MinGw" || ${machine} == "CYGWIN" ]]; then
    echo "Using OS: Windows"
    bazel build --config=vs2019-prod //subtitler/gui:main

    [ -d ${SUBTITE_RELEASE_PATH} ] && rm -r ${SUBTITE_RELEASE_PATH}

    mkdir -p ${SUBTITE_RELEASE_PATH}
    cp bazel-bin/subtitler/gui/*.dll ${SUBTITE_RELEASE_PATH}/
    cp bazel-bin/subtitler/gui/main.exe ${SUBTITE_RELEASE_PATH}/subtite.exe
    mkdir -p ${SUBTITE_RELEASE_PATH}/plugins
    cp -r ${QT5_INSTALL_PATH}/plugins/audio ${SUBTITE_RELEASE_PATH}/plugins/audio
    cp -r ${QT5_INSTALL_PATH}/plugins/platforms ${SUBTITE_RELEASE_PATH}/plugins/platforms
    ResourceHacker \
        -open ${SUBTITE_RELEASE_PATH}/subtite.exe \
        -save ${SUBTITE_RELEASE_PATH}/subtite-icon.exe \
        -action addskip \
        -res subtitler/gui/resource/images/logo.ico \
        -mask ICONGROUP,MAINICON,
    mv ${SUBTITE_RELEASE_PATH}/subtite-icon.exe ${SUBTITE_RELEASE_PATH}/subtite.exe

elif [[ ${machine} == "Linux" ]]; then
    echo "Using OS: Linux"
    bazel build --config=gcc-prod //subtitler/gui:main
    cp -r deploy/linux/usr ${SUBTITE_RELEASE_PATH}
    # TODO
else
    echo "Unsupported OS: ${machine}"
    exit 1
fi

echo "Deployed to ${SUBTITE_RELEASE_PATH}"
