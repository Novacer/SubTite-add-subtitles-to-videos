#ifndef SUBTITLER_SUBPROCESS_SUBPROCESS_EXECUTOR_H
#define SUBTITLER_SUBPROCESS_SUBPROCESS_EXECUTOR_H

#include <memory>
#include <optional>
#include <string>
#include <functional>

namespace subtitler {
namespace subprocess {

/**
 * SubprocessExecutor serves as an abstraction to asynchronously run a
 * subprocess. Stdout is collected and returned to the caller as a string after
 * termination. The client may have multiple instances of SubprocessExecutor
 * alive at the same time.
 *
 * Sample Usage:
 * SubprocessExecutor subprocess;
 * subprocess.SetCommand("echo hello world");
 * subprocess.CaptureOutput(true);
 * subprocess.Start()
 * // Do some work in the meantime...
 * auto captured_output = subprocess.WaitUntilFinished();
 */
class SubprocessExecutor {
  public:
    SubprocessExecutor();
    SubprocessExecutor(const std::string &command, bool capture_output);
    virtual ~SubprocessExecutor();

    // Sets the command to be used upon Start().
    virtual void SetCommand(const std::string &command);

    // Sets the callback to be called as repeatedly as data is read from stdout.
    virtual void SetCallback(std::function<void(const char *)> callback);

    // Sets whether stdout and stderr should be captured.
    virtual void CaptureOutput(bool capture_output);

    // Start executing the command. Throws std::runtime_error if unable to
    // Start() the command.
    virtual void Start();

    struct Output {
        std::string subproc_stdout;
        std::string subproc_stderr;
    };

    // Wait until process finishes and return its stdout and stderr.
    // If capture output is set false, then returns empty string.
    // If timeout is not set then wait forever.
    // If timeout is set, then wait at most 2 * timeout_ms before force
    // terminating the process.
    virtual Output WaitUntilFinished(
        std::optional<int> timeout_ms = std::nullopt);

  private:
    std::string command_;
    bool capture_output_;
    bool is_running_;
    std::function<void(const char *)> callback_;

    struct PlatformDependentFields;
    std::unique_ptr<PlatformDependentFields> fields;
};

}  // namespace subprocess
}  // namespace subtitler

#endif
