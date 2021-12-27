#ifndef SUBTITLER_SUBPROCESS_SUBPROCESS_EXECUTOR_H
#define SUBTITLER_SUBPROCESS_SUBPROCESS_EXECUTOR_H

#include <string>
#include <memory>

namespace subtitler {
namespace subprocess {

/**
 * SubprocessExecutor serves as an abstraction to asynchronously run a subprocess.
 * Stdout is collected and returned to the caller as a string after termination. 
 * The client may have multiple instances of SubprocessExecutor alive at the same time.
 * 
 * Sample Usage:
 * SubprocessExecutor subprocess;
 * subprocess.SetCommand("echo hello world");
 * subprocess.CaptureStdout(true);
 * subprocess.Start()
 * std::string captured_stdout = subprocess.WaitUntilFinished();
 */
class SubprocessExecutor {
public:
    SubprocessExecutor();
    SubprocessExecutor(const std::string &command, bool capture_stdout);
    ~SubprocessExecutor();

    // Sets the command to be used upon Start().
    void SetCommand(const std::string &command);

    // Sets whether stdout should be captured.
    void CaptureStdout(bool capture_stdout);

    // Start executing the command. Throws std::runtime_error if unable to Start() the command.
    void Start();

    // Waits for the subprocess to finish, then returns its stdout.
    // If capture_stdout was set false, then empty string is returned.
    std::string WaitUntilFinished();

private:
    std::string command_;
    bool capture_stdout_;
    bool is_running_;

    struct PlatformDependentFields;
    std::unique_ptr<PlatformDependentFields> fields;
};

} // namespace subtitler
} // namespace subprocess

#endif
