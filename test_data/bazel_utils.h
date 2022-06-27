#ifndef TEST_DATA_BAZEL_UTILS_H
#define TEST_DATA_BAZEL_UTILS_H

#include <string>

namespace test_data {

/**
 * @brief Allows a test binary to access its runfiles.
 * Any file contained in the data atrribute of a target is available
 * to be accessed during runtime. However, the path of the file is managed
 * by bazel build system through symlinks and other methods. This is a cross-
 * platform way to extract an absolute path to the runfile, given the relative
 * path in the workspace.
 *
 * For example, to access mcs_speech_to_text.json, one would query
 * GetBazelDataAbsolutePath("__main__/test_data/mcs_speech_to_text.json")
 *
 * Note that __main__ is the default workspace name. If in the WORKSPACE file
 * there is a user provided name then it will be set to that.
 *
 * https://github.com/bazelbuild/bazel/blob/master/tools/cpp/runfiles/runfiles_src.h
 *
 * @param relative_path the relative path in the workspace (see above).
 * @return std::string the absolute path to access the file.
 */
std::string GetBazelDataAbsolutePath(const std::string& relative_path);

}  // namespace test_data

#endif
