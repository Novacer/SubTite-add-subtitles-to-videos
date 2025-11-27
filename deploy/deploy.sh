#!/bin/bash

# Build Release version of SubTite GUI.
# This script should be run from the project root 
# ie you must invoke with ./deploy/deploy.sh

if [[ -z ${SUBTITE_RELEASE_PATH} ]]; then
    echo "Please set SUBTITE_RELEASE_PATH env variable to be the directory to copy release binary"
    exit 1
else
    echo "Using SUBTITE_RELEASE_PATH: ${SUBTITE_RELEASE_PATH}"
fi

CUR_PROJECT_ROOT=$(pwd)

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

set -e

if [[ ${machine} == "MinGw" || ${machine} == "CYGWIN" ]]; then
    echo "Using OS: Windows"
    if [[ -z ${QT5_INSTALL_PATH} ]]; then
        echo "Please set QT5_INSTALL_PATH env variable to be the QT5 path containing lib, bin, plugins etc."
        exit 1
    else
        echo "Using QT5 Path: ${QT5_INSTALL_PATH}"
    fi

    bazel build --config=vs2019-prod //subtitler/gui:main

    SUBTITE_RELEASE_PATH=${SUBTITE_RELEASE_PATH}/Subtite-win_x86_64
    [ -d ${SUBTITE_RELEASE_PATH} ] && rm -r ${SUBTITE_RELEASE_PATH}

    mkdir -p ${SUBTITE_RELEASE_PATH}
    cp bazel-bin/subtitler/gui/*.dll ${SUBTITE_RELEASE_PATH}/
    cp bazel-bin/subtitler/gui/main.exe ${SUBTITE_RELEASE_PATH}/subtite.exe
    cp bazel-$(basename ${CUR_PROJECT_ROOT})/external/ffmpeg_windows/bin/ffmpeg.exe ${SUBTITE_RELEASE_PATH}/ffmpeg.exe
    mkdir -p ${SUBTITE_RELEASE_PATH}/plugins
    cp -r ${QT5_INSTALL_PATH}/plugins/audio ${SUBTITE_RELEASE_PATH}/plugins/audio
    cp -r ${QT5_INSTALL_PATH}/plugins/platforms ${SUBTITE_RELEASE_PATH}/plugins/platforms
    MCS_INSTALL_LOCATION="C:/Program Files/PackageManagement/NuGet/Packages/Microsoft.CognitiveServices.Speech.1.47.0"
    cp "${MCS_INSTALL_LOCATION}/runtimes/win-x64/native/Microsoft.CognitiveServices.Speech.extension".* ${SUBTITE_RELEASE_PATH}
    ResourceHacker \
        -open ${SUBTITE_RELEASE_PATH}/subtite.exe \
        -save ${SUBTITE_RELEASE_PATH}/subtite-icon.exe \
        -action addskip \
        -res subtitler/gui/resource/images/logo.ico \
        -mask ICONGROUP,MAINICON,
    mv -f ${SUBTITE_RELEASE_PATH}/subtite-icon.exe ${SUBTITE_RELEASE_PATH}/subtite.exe
    cp LICENSE.md ${SUBTITE_RELEASE_PATH}/

elif [[ ${machine} == "Linux" ]]; then
    echo "Using OS: Linux"
    bazel build --config=gcc-prod //subtitler/gui:main

    SUBTITE_RELEASE_PATH=${SUBTITE_RELEASE_PATH}/Subtite-linux_x86_64
    [ -d ${SUBTITE_RELEASE_PATH} ] && rm -r ${SUBTITE_RELEASE_PATH}

    mkdir -p ${SUBTITE_RELEASE_PATH}
    cp LICENSE.md ${SUBTITE_RELEASE_PATH}/
    cp -r deploy/linux/usr ${SUBTITE_RELEASE_PATH}
    mkdir -p ${SUBTITE_RELEASE_PATH}/usr/bin
    cp bazel-bin/subtitler/gui/main ${SUBTITE_RELEASE_PATH}/usr/bin/subtite
    cp bazel-$(basename ${CUR_PROJECT_ROOT})/external/ffmpeg_linux/bin/ffmpeg ${SUBTITE_RELEASE_PATH}/usr/bin/ffmpeg
    mkdir -p ${SUBTITE_RELEASE_PATH}/usr/lib

    cd ${SUBTITE_RELEASE_PATH}
    LD_LIBRARY_PATH=${CUR_PROJECT_ROOT}/bazel-$(basename ${CUR_PROJECT_ROOT})/external/ffmpeg_linux/lib \
	linuxdeployqt \
	${SUBTITE_RELEASE_PATH}/usr/share/applications/subtite.desktop \
	-executable=${SUBTITE_RELEASE_PATH}/usr/bin/ffmpeg \
	-appimage
    mv Subtite-x86_64.AppImage Subtite-linux_x86_64.AppImage
    zip Subtite-linux_x86_64.zip Subtite-linux_x86_64.AppImage LICENSE.md

else
    echo "Unsupported OS: ${machine}"
    exit 1
fi

echo "Deployed to ${SUBTITE_RELEASE_PATH}"
