#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <future>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/util/unicode.h"

namespace subtitler {
namespace subprocess {

namespace {

const int BUFFER_SIZE = 1024;

std::string PollHandle(int fd) {
    int bytes_read = 0;
    char buffer[BUFFER_SIZE + 1];
    std::ostringstream str;

    // Spin until we are unable to read the pipe anymore
    for (;;) {
        bytes_read = read(
            /* fd= */ fd,
            /* buf= */ buffer,
            /* count= */ BUFFER_SIZE);
        if (bytes_read < 0 || bytes_read == 0) {
            break;
        }
        // Ensure null termiantion
        buffer[amount_read] = '\0';
        str << buffer;
    }

    return str.str();
}

void WaitTimeoutOrKill(pid_t pid, int timeout_ms) {
    int status = 0;
    // Check if already terminated
    auto ret = waitpid(pid, &status, WNOHANG);
    if (ret < 0) {
        throw std::runtime_error("waitpid() error");
    }
    if (ret == 0) {
        // Didn't see it terminate.
        // Wait timeout / 2
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms / 2));
        // Try again
        ret = waitpid(pid, &status, WNOHANG);
        if (ret < 0) {
            throw std::runtime_error("waitpid() error after timeout");
        }
        if (ret == 0) {
            // Still nothing... send terminate.
            ret = kill(pid, SIGTERM);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(timeout_ms / 2));
            ret = waitpid(pid, &status, WNOHANG);
            if (ret < 0) {
                throw std::runtime_error("waitpid() error after SIGTERM");
            }
            if (ret == 0) {
                // Not responding to terminate
                // Hard kill it.
                kill(pid, SIGKILL);
                ret = waitpid(pid, &status, 0);
                if (ret < 0) {
                    throw std::runtime_error("waitpid() error after SIGKILL");
                }
            }
        }
    }
}

}  // namespace

struct SubprocessExecutor::PlatformDependentFields {
    int stderr_fd = 0;
    int stdout_fd = 0;
    pid_t pid = 0;
    std::unique_ptr<posix_spawn_file_actions_t> actions = nullptr;
    std::unique_ptr<std::future<std::string>> captured_output = nullptr;
    std::unique_ptr<std::future<std::string>> captured_error = nullptr;
};

SubprocessExecutor::SubprocessExecutor()
    : command_{},
      capture_output_{false},
      is_running_{false},
      fields{std::make_unique<PlatformDependentFields>()} {}

SubprocessExecutor::SubprocessExecutor(const std::string &command,
                                       bool capture_output)
    : command_{command},
      capture_output_{capture_output},
      is_running_{false},
      fields{std::make_unique<PlatformDependentFields>()} {}

SubprocessExecutor::~SubprocessExecutor() {
    if (is_running_ && fields->pid > 0) {
        // Force kill other process.
        // No throw and no block so it's "safe" to call in dtor.
        kill(fields->pid, SIGKILL);
        // Dtor of future will block until both threads terminate.
        // Since we have killed the other process, this should terminate
        // eventually.
        fields->captured_output.reset();
        fields->captured_error.reset();

        close(fields->stdout_fd);
        fields->stdout_fd = 0;
        close(fields->stderr_fd);
        fields->stderr_fd = 0;
        fields->pid = 0;
        posix_spawn_file_actions_destroy(fields->actions.get());
        fields->actions.reset();
    }
}

void SubprocessExecutor::SetCommand(const std::string &command) {
    command_ = command;
}

void SubprocessExecutor::CaptureOutput(bool capture) {
    capture_output_ = capture;
}

void SubprocessExecutor::Start() {
    if (is_running_) {
        throw std::runtime_error(
            "You must call WaitUntilFinished() before starting again.");
    }
    // pipe[0] is for our side, pipe[1] is for subprocess side.
    int cout_pipe[2];
    int cerr_pipe[2];
    // save as unique ptr?
    auto action = std::make_unique<posix_spawn_file_actions_t>();
    if (pipe(cout_pipe) < 0) {
        throw std::runtime_error("Could not create stdout pipe");
    }
    if (pipe(cerr_pipe) < 0) {
        throw std::runtime_error("Could not create stderr pipe");
    }
    if (!posix_spawn_file_actions_init(action.get())) {
        throw std::runtime_error("Error while initializing subprocess");
    }
    // Tell spawned process to close our side of the pipes
    if (!posix_spawn_file_actions_addclose(action.get(), cout_pipe[0])) {
        throw std::runtime_error("Error while configuring subprocess");
    }
    if (!posix_spawn_file_actions_addclose(action.get(), cerr_pipe[0])) {
        throw std::runtime_error("Error while configuring subprocess");
    }
    // Tell spawned process to make a duplicate of their end of the pipe.
    if (!posix_spawn_file_actions_adddup2(action.get(), cout_pipe[1], 1)) {
        throw std::runtime_error("Error while configuring subprocess");
    }
    if (!posix_spawn_file_actions_adddup2(action.get(), cerr_pipe[1], 2)) {
        throw std::runtime_error("Error while configuring subprocess");
    }
    // Tell spanwed process to close their (old) end of the pipe.
    if (!posix_spawn_file_actions_addclose(action.get(), cout_pipe[1])) {
        throw std::runtime_error("Error while configuring subprocess");
    }
    if (!posix_spawn_file_actions_addclose(action.get(), cerr_pipe[1])) {
        throw std::runtime_error("Error while configuring subprocess");
    }

    // We run sh -c {command}, so we don't have to worry about arg splitting.
    // Using an array is a workaround so we can get non-const ptr later.
    std::string sh_prefix[] = {"sh", "-c"};
    char *args[] = {&sh_prefix[0][0],  // sh
                    &sh_prefix[1][0],  // -c
                    &command_[0],      // {command}
                    nullptr};
    pid_t pid = 0;
    if (posix_spawnp(&pid, args[0], action.get(), nullptr, args, nullptr)) {
        throw std::runtime_error("Failed to spawn subprocess");
    }

    // Close pipe ends on subprocess' side.
    close(cout_pipe[1]);
    close(cerr_pipe[1]);

    is_running_ = true;
    // Store needed fields
    fields->stdout_fd = cout_pipe[0];
    fields->stderr_fd = cerr_pipe[0];
    fields->pid = pid;
    fields->actions = std::move(action);

    if (capture_output_) {
        fields->captured_output = std::make_unique<std::future<std::string>>(
            std::async(std::launch::async,
                       [this] { return PollHandle(fields->stdout_fd); }));
        fields->captured_error = std::make_unique<std::future<std::string>>(
            std::async(std::launch::async,
                       [this] { return PollHandle(fields->stderr_fd); }));
    }
}

SubprocessExecutor::Output SubprocessExecutor::WaitUntilFinished(
    std::optional<int> timeout_ms) {
    if (!is_running_) {
        throw std::runtime_error(
            "You must call Start() before you are able to wait.");
    }

    if (timeout_ms) {
        // Wait with timeout
        WaitTimeoutOrKill(fields->pid, *timeout_ms);
    } else {
        int status = 0;
        // Wait forever
        waitpid(-1, &status, 0);
    }

    SubprocessExecutor::Output output;
    if (fields->captured_output) {
        // Block until stdout thread finishes
        output.subproc_stdout = fields->capture_output_->get();
    }
    if (fields->captured_error) {
        // Block until stdout thread finishes
        output.subproc_stderr = fields->captured_error->get();
    }

    is_running_ = false;

    // Cleanup everything
    close(fields->stdout_fd);
    fields->stdout_fd = 0;
    close(fields->stderr_fd);
    fields->stderr_fd = 0;
    fields->pid = 0;
    posix_spawn_file_actions_destroy(fields->actions.get());
    fields->actions.reset();
    fields->captured_output.reset();
    fields->captured_error.reset();
}

}  // namespace subprocess
}  // namespace subtitler
