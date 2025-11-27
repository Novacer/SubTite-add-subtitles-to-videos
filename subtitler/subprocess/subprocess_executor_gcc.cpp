#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wordexp.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "subtitler/subprocess/subprocess_executor.h"
#include "subtitler/util/unicode.h"

// From <unistd.h>, defines the current environment variables.
// Can be passed to posix_spawnp to inherit the same environment.
extern char** environ;

namespace subtitler {
namespace subprocess {

namespace {

const int BUFFER_SIZE = 1024;

std::string PollHandle(int fd, bool return_output,
                       std::function<void(const char*)> callback) {
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
    buffer[bytes_read] = '\0';

    if (callback) {
      callback(buffer);
    }
    if (return_output) {
      str << buffer;
    }
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
      std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms / 2));
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
  int stderr_fd = -1;
  int stdout_fd = -1;
  pid_t pid = -1;
  std::unique_ptr<posix_spawn_file_actions_t> actions = nullptr;
  std::unique_ptr<std::future<std::string>> captured_output = nullptr;
  std::unique_ptr<std::future<std::string>> captured_error = nullptr;
};

SubprocessExecutor::SubprocessExecutor()
    : command_{},
      capture_output_{false},
      is_running_{false},
      fields{std::make_unique<PlatformDependentFields>()} {}

SubprocessExecutor::SubprocessExecutor(const std::string_view command,
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
    fields->stdout_fd = -1;
    close(fields->stderr_fd);
    fields->stderr_fd = -1;
    fields->pid = -1;
    posix_spawn_file_actions_destroy(fields->actions.get());
    fields->actions.reset();
    callback_ = {};
  }
}

void SubprocessExecutor::SetCommand(const std::string_view command) {
  command_ = command;
}

void SubprocessExecutor::SetCallback(
    std::function<void(const char*)> callback) {
  callback_ = callback;
}

void SubprocessExecutor::CaptureOutput(bool capture) {
  capture_output_ = capture;
}

void SubprocessExecutor::Start() {
  if (is_running_) {
    throw std::runtime_error(
        "You must call WaitUntilFinished() before starting again.");
  }
  if (std::all_of(command_.begin(), command_.end(), isspace)) {
    throw std::runtime_error("Cannot start process with empty command!");
  }

  // pipe[0] is for our side, pipe[1] is for subprocess side.
  int cout_pipe[2] = {-1};
  int cerr_pipe[2] = {-1};
  auto action = std::make_unique<posix_spawn_file_actions_t>();

  if (capture_output_ || callback_) {
    if (pipe(cout_pipe) < 0) {
      throw std::runtime_error("Could not create stdout pipe");
    }
    if (pipe(cerr_pipe) < 0) {
      throw std::runtime_error("Could not create stderr pipe");
    }
    if (posix_spawn_file_actions_init(action.get())) {
      throw std::runtime_error("Error while initializing subprocess");
    }
    // Tell spawned process to close our side of the pipes
    if (posix_spawn_file_actions_addclose(action.get(), cout_pipe[0])) {
      throw std::runtime_error("Error while configuring subprocess");
    }
    if (posix_spawn_file_actions_addclose(action.get(), cerr_pipe[0])) {
      throw std::runtime_error("Error while configuring subprocess");
    }
    // Tell spawned process to make a duplicate of their end of the pipe.
    if (posix_spawn_file_actions_adddup2(action.get(), cout_pipe[1], 1)) {
      throw std::runtime_error("Error while configuring subprocess");
    }
    if (posix_spawn_file_actions_adddup2(action.get(), cerr_pipe[1], 2)) {
      throw std::runtime_error("Error while configuring subprocess");
    }
    // Tell spanwed process to close their (old) end of the pipe.
    if (posix_spawn_file_actions_addclose(action.get(), cout_pipe[1])) {
      throw std::runtime_error("Error while configuring subprocess");
    }
    if (posix_spawn_file_actions_addclose(action.get(), cerr_pipe[1])) {
      throw std::runtime_error("Error while configuring subprocess");
    }
  }

  // Expands command str into an array of args
  // Ex: foo bar "baz ham" -> ["foo", "bar", "baz ham"] etc.
  wordexp_t arg_expansion;
  if (wordexp(command_.c_str(), &arg_expansion, WRDE_NOCMD)) {
    throw std::runtime_error("Could not expand command!");
  }

  pid_t pid = 0;
  if (posix_spawnp(&pid, arg_expansion.we_wordv[0], action.get(), nullptr,
                   arg_expansion.we_wordv, environ)) {
    wordfree(&arg_expansion);
    posix_spawn_file_actions_destroy(action.get());
    close(cout_pipe[0]), close(cout_pipe[1]);
    close(cerr_pipe[0]), close(cerr_pipe[1]);
    throw std::runtime_error("Unable to create process to run: " + command_);
  }
  wordfree(&arg_expansion);

  // Close pipe ends on subprocess' side.
  close(cout_pipe[1]);
  close(cerr_pipe[1]);

  is_running_ = true;
  // Store needed fields
  fields->stdout_fd = cout_pipe[0];
  fields->stderr_fd = cerr_pipe[0];
  fields->pid = pid;
  fields->actions = std::move(action);

  if (capture_output_ || callback_) {
    fields->captured_output = std::make_unique<std::future<std::string>>(
        std::async(std::launch::async, [this] {
          return PollHandle(fields->stdout_fd, capture_output_, callback_);
        }));
    fields->captured_error = std::make_unique<std::future<std::string>>(
        std::async(std::launch::async, [this] {
          return PollHandle(fields->stderr_fd, capture_output_, {});
        }));
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
    waitpid(fields->pid, &status, 0);
  }

  SubprocessExecutor::Output output;
  if (fields->captured_output) {
    // Block until stdout thread finishes
    output.subproc_stdout = fields->captured_output->get();
  }
  if (fields->captured_error) {
    // Block until stderr thread finishes
    output.subproc_stderr = fields->captured_error->get();
  }

  is_running_ = false;

  // Cleanup everything
  close(fields->stdout_fd);
  fields->stdout_fd = -1;
  close(fields->stderr_fd);
  fields->stderr_fd = -1;
  fields->pid = -1;
  posix_spawn_file_actions_destroy(fields->actions.get());
  fields->actions.reset();
  fields->captured_output.reset();
  fields->captured_error.reset();
  callback_ = {};

  return output;
}

}  // namespace subprocess
}  // namespace subtitler
