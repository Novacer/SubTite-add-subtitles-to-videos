# SubTite: Easy way to create and add subtitles for your videos

![Latest Build Status](https://github.com/Novacer/SubTite-add-subtitles-to-videos/actions/workflows/main.yml/badge.svg?branch=master)

**TLDR:** SubTite allows you to create subtitles using auto-transcription and edit them seamlessly in a video editor. The subtitles can be output as a separate SRT file, or combined with the video. Existing SRT files can be imported and edited effortlessly. The subtitles can be positioned in 9 different locations along the video. Support for trimming video, adding images, and other video editing features coming soon!

## GUI Demo
![gif demo](https://github.com/Novacer/SubTite-add-subtitles-to-videos/assets/29148427/c2afaac0-5111-4f7e-ad38-69b7f9757ad3)

## Feature Overview (Current v1.0.4)
* Supports Windows and Linux
* GUI + CLI for adding subtitles
* Subtitle Auto-transcription through MS Cognitive Services (windows only for now)
* Intuitive interface to make subtitling as fast and pain-free as possible
* Full UTF-8 Unicode support on Windows and Linux
* Subtitles can be positioned in 9 different locations: top right, middle center, bottom left etc.
* Added subtitles can immediately be previewed in a video player on the fly
* Existing subtitles (.srt) can be loaded and easily edited
* Video with subtitles can be exported either by remuxing to mkv or burning subtitles into the mp4.

## Installation

### To use the GUI
Download the appropriate binaries from the [releases section](https://github.com/Novacer/SubTite-add-subtitles-to-videos/releases)

### To use the CLI
Please refer to the [CLI docs](subtitler/cli/README.md)

## Usage

### Subtitle Editing
1. Select the video from the file dialog.
2. Select the subtitle file from the next file dialog. You can create a new subtitle file to start from scratch, or use an existing (.srt) file.
3. Position the yellow indicator at the subtitle's start position.
4. Right-click and select "Add interval at position".
5. Enter the subtitle text on the pane to the right.
6. (Optional) Select the subtitle position from one of the 9 positions.
7. Use mouse to adjust the subtitle's start and end time. Any changes are saved automatically.
8. Select `File > Export` if you want to combine the video and subtitles!

### Auto Transcription (Windows only for now)
1. Create a free [Microsoft Azure Account](https://azure.microsoft.com/en-us/free/cognitive-services/) (although it is free, they will require a credit card).
2. Create a [free speech resource](https://ms.portal.azure.com/#create/Microsoft.CognitiveServicesSpeechServices) in the Azure portal. Make sure to pick the free (F0) pricing tier! As of time of writing, this should give you 5 hours of free subtitle transcription per month.
3. Now you should have 2 API keys under "Keys and Endpoint" on the Azure portal. Copy one of them.
4. In subtite, select `Subtitle > Auto Transcribe`. Copy in your keys and also the Region of your speech instance. (For example, westus)
5. Enter a password to remember this information for next time. The password is used to encrypt the API keys to store on your local disk, and it is never passed anywhere else except to the Microsoft servers.
6. Choose the output location where the auto-transcribed subtitles will be saved.
7. Wait for your audio to be processed for auto-transcription!

## Building from source
SubTite uses bazel as the main build system. Setup bazel on your environment following this link: https://docs.bazel.build/versions/main/install.html.

### Windows
Compiling for windows requires MSVC 2019.

With bazel setup, here are some sample commands for building the CLI.

```bash
$ bazel build --config=vs2019-prod //subtitler/cli:cli # Build CLI in release mode using MSVC2019
```

Building the GUI requires QT 5 to be installed on your machine. On windows specifically, we expect the path to be somewhat similar to
```
C:\Qt\5.15.2\msvc2019_64\... etc
```

We also require the Microsoft Speech Services SDK to be installed. You may install it following the
[microsoft documentation](https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/quickstarts/setup-platform?tabs=windows%2Cubuntu%2Cdotnet%2Cjre%2Cmaven%2Cnodejs%2Cmac%2Cpypi&pivots=programming-language-cpp). We expect the SDK to be installed at a path similar to
```
C:\\Program Files\\PackageManagement\\NuGet\\Packages\\Microsoft.CognitiveServices.Speech.1.22.0
```

Assuming you have the same paths as above, subtite can be easily built with

```bash
./build_gui_dev.sh
```

It is recommended to use the build script instead of building with `bazel build ...`, as the build script copies in additional runtime dependencies (qt audio plugins etc.) which are required to run SubTite. You may need to set `QT5_INSTALL_PATH` env variable to be something like `C:\\Qt\\5.15.2\\msvc2019_64\\`

If for some reason the build script doesn't work for you, here's how to build manually on Windows.

---

First `bazel build --config=vs2019-prod //subtitler/gui:main`

The binaries and dynamic libraries are contained in `bazel-bin/subtitler/gui/`. In order to have audio played in the integrated player on windows, you need to copy the QT audio plugins into this folder. In particular, you need to create the folder `bazel-bin/subtitler/gui/plugins`. Then, copy the audio folder from `C:\Qt\5.15.2\msvc2019_64\plugins\` into `bazel-bin/subtitler/gui/plugins`.

Next, you need to copy the Microsoft Speech Service runtime dlls into `bazel-bin/subtitler/gui/`. For example, this is locationed somewhere like
```
C:/Program Files/PackageManagement/NuGet/Packages/Microsoft.CognitiveServices.Speech.1.22.0/runtimes/win-x64/native/
```

Finally, you want to copy ffmpeg.exe into `bazel-bin/subtitler/gui/`. When you ran bazel build, a copy of ffmpeg.exe is already downloaded to `bazel-subtitler/external/ffmpeg_windows/bin/ffmpeg.exe`. You may copy that or any other ffmpeg binary you obtain elsewhere.

### Linux

Compiling for linux requires GCC.

With bazel setup, here are some sample commands for building the CLI.
```bash
$ bazel build --config=gcc-prod //subttiler/cli:cli    # Build CLI in release mode using GCC
```

To compile the GUI, you will need to install various dependencies. 

```bash
sudo apt-get install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools \
  qtmultimedia5-dev qttools5-dev-tools \
  libva-dev libsodium-dev
```

This should install QT 5 to one of the following locations.

```
/usr/include/x86_64-linux-gnu/qt5
or
/usr/include/qt
```

Now SubTite can be built all from one command:

```bash
bazel clean --expunge && ./build_gui_dev.sh
```
It is recommended to use the build script instead of compiling manually, since the build script ensures the correct runtime dependencies are copied to the right places. If the script doesn't work for you, here's how to compile manually.

---
First, `bazel build --config=gcc-prod //subtitler/gui:main`

Next, copy `bazel-subtitler/external/ffmpeg_linux/bin/ffmpeg` into `bazel-bin/subtitler/gui`. You should be ready to go from here.

## Build configs

The following build options are configured in [.bazelrc](https://github.com/Novacer/SubTite-add-subtitles-to-videos/blob/master/.bazelrc) of this project:
1. `--config=vs2019-prod`. MSVC release mode
2. `--config=vs2019-asan`. MSVC debug mode, with address sanitizer
3. `--config=vs2019`. MSVC with bazel default settings (fastest build times)

`--config=gcc-prod`, `--config=gcc-asan`, and `--config=gcc` are the same as above, except it is for building on Linux using gcc.

Unit tests can be run using
```bash
$ bazel test --config=vs2019-asan ... # used for running on windows
$ bazel test --config=gcc-asan ...    # used for running on linux
```

## Deploying - Packaging Subtite with it's dependencies.
Since Subtite uses Dynamic Linking with QT and FFMPEG (so it can remain LGPL compliant with those projects), there is a need to package the dependencies of Subtite when deploying. A deploy script can be run `./deploy/deploy.sh` (IMPORTANT: script has to be run from the project root) which will package the dependecies automatically.

In addition to the dependencies required to build Subtite, deploying also requires [Resource Hacker](http://www.angusj.com/resourcehacker/) on Windows, and [linuxdeployqt](https://github.com/probonopd/linuxdeployqt) on Linux.

On Windows only, the script requires setting `QT5_INSTALL_PATH` env variable to the directory where QT 5 is installed. This should look something like `C:\\Qt\\5.15.2\\msvc2019_64\\`.

On both Windows and Linux, the script requires setting `SUBTITE_RELEASE_PATH` which is the path to the output directory that you want to write the deployed binaries to.

## Experimental
Some experimental binaries are also placed in the `subtitler/experimental` folder.
These contain some methods which are not ready to use in production but are interesting demos.

### Trimmer

`subtitler/experimental/trimmer_msvc.cpp` implements trimming of a video on windows.
You select a video, a timestamp file, and an output video path.

The timestamp file is of the syntax:

```
00:00:00 - 00:01:00
00:03:00 - 00:03:30
... etc ...
```
Will produce an output video with **only** the sections from `0s - 1min` and `3min - 3min 30s` of the original input video.

Trimmer can be built using
```
$ bazel build --config=vs2019 //subtitler/experimental/trimmer:trimmer
```

This functionality will eventually be integrated in the SubTite binary.

### Subtitle Burner (In production)
`subtitler/experimental/sub_burner.cpp` demonstrates how subtitles can be permanently burned into a video. It requires as input 3 command line flags:

```
--video_path: path to the input video file
--subtitle_path: path to the SRT subtitle file
--output_path: path to where the output file will be written
```

SubBurner can be built using
```
$ bazel build --config=vs2019 //subtitler/experimental/sub_burner:sub_burner
```

## Planned Features
### V1.1-2.0
* Support for subtitles of various fonts and colours
* Support simple video editing, such as cropping/trimming
* Support adding static images on top of the video between certain timestamps.
* Support for displaying audio waveforms along the timeline (similar to Audacity etc).

# Licensing
SubTite is available under [The Prosperity Public License 3.0.0](https://prosperitylicense.com/versions/3.0.0)
* If you use SubTite in a non-commercial manner, then you have permission to do so for free.
* If you use SubTite in any commercial manner, then you are subject to a 30-day free trial. After which, you must initiate contact directly to discuss a commercial licensing agreement.

## Attribution - SubTite uses the following projects:
### LGPL
* [FFMPEG](https://www.ffmpeg.org/legal.htmlhttps://www.ffmpeg.org/legal.html). Pre-built shared libraries are obtained from https://github.com/BtbN/FFmpeg-Builds/releases
* [Qt](https://doc.qt.io/qt-5/lgpl.html) 

### MIT
* [nlohmann json](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)
* [howard hinnant date](https://github.com/HowardHinnant/date/blob/master/LICENSE.txt)
* [QT AV Player](https://github.com/Novacer/QtAVPlayer/blob/master/LICENSE)
* [csuft video timeline](https://github.com/csuft/VideoTimeline/blob/68df754ce0385e8f6d1f67cd8ee2dd0f9fcf67df/readme.md)

### BSD 3
* [gflags](https://github.com/gflags/gflags/blob/master/COPYING.txt)
* [googletest](https://github.com/google/googletest/blob/main/LICENSE)

### Other
* [glog](https://github.com/google/glog/blob/master/COPYING)
* [bazel rules qt](https://github.com/Novacer/bazel_rules_qt/blob/master/LICENSE)
* [libsodium](https://github.com/jedisct1/libsodium/blob/master/LICENSE)
* [MS Speech Service SDK](https://docs.microsoft.com/en-us/azure/cognitive-services/speech-service/)
