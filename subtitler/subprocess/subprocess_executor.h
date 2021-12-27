#ifndef SUBTITLER_SUBPROCESS_SUBPROCESS_EXECUTOR_H
#define SUBTITLER_SUBPROCESS_SUBPROCESS_EXECUTOR_H

#include <string>
#include <memory>

// TODO: documentation.

namespace subtitler {
namespace subprocess {

class SubprocessExecutor {
public:
    SubprocessExecutor();
    ~SubprocessExecutor();

    void SetCommand(const std::string& command);
    void CaptureStdout(bool capture);
    void Start();
    std::string WaitUntilFinished();

private:
    std::string command_;
    bool capture_stdout_;

    struct PlatformDependentFields;
    std::unique_ptr<PlatformDependentFields> fields;
};

} // namespace subtitler
} // namespace subprocess

#endif
