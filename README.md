# SubTite: Easy way to create and add subtitles for your videos
**TLDR:** SubTite allows you to add subtitles and preview it immediately in the video player. The subtitles will be output as a separate SRT file. Existing SRT files can be imported and edited efforlessly. The subtitles can be positioned in 9 different locations along the video. Support for adding images and trimming video coming soon!

![demo](https://user-images.githubusercontent.com/29148427/151613092-e7dcf2c3-80dd-4f72-a3c9-8bdd220594b8.gif)


## Feature Overview (Current v0.1)
* Supports Windows and Linux
* CLI for adding subtitles
* Intuitive interface to make subtitling as fast and pain-free as possible
* Full UTF-8 Unicode support on Windows and Linux
* Subtitles can be positioned in 9 different locations: top right, middle center, bottom left etc.
* Added subtitles can immediately be previewed in a video player on the fly
* Existing subtitles can be loaded and easily edited

## Installation
### To use the CLI
The CLI edition of SubTite is the Keep-It-Simple solution to quickly add subtitles without being bogged down by UI elements.

### Linux
1. `sudo apt install ffmpeg`. Ensure the following works:
   * `ffmpeg -version`
   * `ffplay -version`
   * `ffprobe -version`
2. Obtain SubTite cli binary from releases section or build the code yourself (see building section below)

### Windows
1. Install the ffmpeg essential binaries from this link: https://www.gyan.dev/ffmpeg/builds/
2. Extract the binaries to a location of your choice, making sure that **the folder is added to your PATH!!!**. After adding the folder containing the ffmpeg binaries to your PATH, you should be able to run the following in cmd, powershell, or some windows bash shell:
   * `ffmpeg -version`
   * `ffplay -version`
   * `ffprobe -version`
3. Obtain SubTite cli binary from releases section or build the code yourself (see building section below)

## Usage
The following are usage instructions for the CLI. Let `subtite` be the name of the binary.

### Paths setup

On Windows, launching subtite binary will immediately open up 2 file dialogs: the first is to select the video you want to subtitle, and second is to save the output subtitle file path. Selecting an existing subtitle means this subtitle will be loaded and can be edited.

On Linux, file paths can be provided by `--video_path` and `--output_subtitle_path` flags respectively.
```bash
$ subtite --video_path "path/to/video.mp4" --output_subtitle_path "path/to/video.srt"
```

The binary will attempt to auto detect the ffmpeg, ffplay, and ffprobe you installed. This should work as long as you have placed the folder containing these binaries in your `PATH`. Alternatively, you can use flags `--ffmpeg_path`, `--ffprobe_path`, `--ffplay_path` to overwrite these paths. This works for both Windows and Linux.

### Interactive subtitle mode
You are ready to add subtitles when you see the following output.
```
Initialized with start=00:00:00.000 duration=00:00:05.000
Starting interactive mode. Type help for instructions.
```

Use command `help` to see an overview of all available commands. We go over some examples here.

Typing `play` will open a video player to play from 0s to 5s. If you want to adjust this play interval use command `play start {start} duration {duration}` or short form `play s {start} d {duration}`. The syntax for `{start}` and `{duration}` can be seen from following examples.

```
play s 5 d 3.5 means play from 5s to 8.5s
play s 1:23.456 means play from 1min 23.456s up to previous duration
play d 2:1:45 means play from previous start point for duration of 2h 1m 45s.
play next or play n means play the next 5 seconds of video
```

The seconds field in the time syntax supports up to 5 sig figs. That means `1:23:45.678` is OK but `45.6789` is not.

You can print any existing subtitles which will be displayed at the current position.
```
printsubs
1
00:00:01,000 --> 00:00:05,000
test
```

You can add subtitles which will show up at the same start and end time you set using `play`.
Note that the opened player may play a few seconds before your selected time, but your subtitles
should show up precisely at the selected time.

```
add
Enter the subtitles, multiple lines allowed. A blank line (enter) represents end of input.
Use /play to replay the video, /cancel to discard all input.Or, add blank line (enter) immediately to exit out of this mode.
Hello this is a
test using two lines of subtitles

Enter next command:
printsubs
1
00:00:00,000 --> 00:00:05,000
Hello this is a
test using two lines of subtitles

2
00:00:01,000 --> 00:00:05,000
test
```

While adding subtitles, you can enter a single line containing `/play` to replay the current position, or
`/cancel` to discard the input. Entering an empty line (pressing enter) will commit the input.
UTF-8 encoded input is supported.

Subtitle position can be set using `add position {position}` or shortform `add p {position}`. The possible values for position are
```
top-left    top-center    top-right
middle-left middle-center middle-right
bottom-left bottom-center bottom-right
```
Or in short form:
```
tl tc tr
ml mc mr
bl bc br
```

Ex:
```
add p mc
Enter the subtitles, multiple lines allowed. A blank line (enter) represents end of input.
Use /play to replay the video, /cancel to discard all input.Or, add blank line (enter) immediately to exit out of this mode.
top of the world

```
Will cause text `top of the world` to show up in the top-center of the video.

There are a number of remaning commands available. This includes, but not limited to
`delete`-ing existing subtitles, `edit`-ing previously committed subtitles, `save` and `quit`. Again use command `help` to see overview of all these commands. More examples are also available from [reading the unit tests](https://github.com/Novacer/SubTite-add-subtitles-to-videos/blob/master/subtitler/cli/commands_test.cpp).


## Building from source
SubTite uses bazel as the main build system. Setup bazel on your environment following this link: https://docs.bazel.build/versions/main/install.html.

Compiling for windows requires MSVC 2019. Compiling for linux requires GCC.

With bazel setup, here are some sample commands for building the CLI.
```bash
$ bazel build --config=vs2019-prod //subtitler/cli:cli # Build CLI in release mode using MSVC2019
$ bazel build --config=gcc-prod //subttiler/cli:cli    # Build CLI in release mode using GCC
```

The following build options are configured in [.bazelrc](https://github.com/Novacer/SubTite-add-subtitles-to-videos/blob/master/.bazelrc) of this project:
1. `--config=vs2019-prod`. MSVC release mode
2. `--config=vs2019-asan`. MSVC debug mode, with address sanitizer
3. `--config=vs2019`. MSVC with bazel default settings (fastest build times)

`--config=gcc-prod`, `--config=gcc-asan`, and `--config=gcc` are the same as above, except it is for building on Linux using gcc.

Unit tests can be run using
```bash
$ bazel test --config=vs2019 ... # used for running on windows
$ bazel test --config=gcc ...    # used for running on linux
```

## Planned Features
### V0.2-0.9
* Support for various fonts and colours
* Subtitles can be baked directly into the video
* Support simple video editing, such as cropping/trimming
* Support adding static images on top of the video between certain timestamps.

### V1.0
* Full graphical user interface with integrated video player, interval selector etc.
