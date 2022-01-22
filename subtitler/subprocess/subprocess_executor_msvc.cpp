#include "subtitler/subprocess/subprocess_executor.h"

#pragma comment(lib, "User32.lib")

#include <windows.h>

#include <future>
#include <sstream>
#include <stdexcept>

#include "subtitler/util/unicode.h"

namespace subtitler {
namespace subprocess {

namespace {

const int BUFFER_SIZE = 1024;
const int TIMEOUT_MS = 5000;

void CleanupHandle(HANDLE &handle) {
    if (handle) {
        CloseHandle(handle);
        handle = NULL;
    }
}

BOOL CALLBACK SendWMCloseMsg(HWND hwnd, LPARAM lParam) {
    DWORD dwProcessId = 0;
    GetWindowThreadProcessId(hwnd, &dwProcessId);
    if (dwProcessId == lParam) {
        SendMessageTimeoutW(
            /* windowHandle= */ hwnd,
            /* msg= */ WM_CLOSE,
            /* wParam= */ 0,
            /* lParam= */ 0,
            /* fuFlags= */ SMTO_ABORTIFHUNG,
            /* uTimeoutMs= */ TIMEOUT_MS,
            /* lpdwResult= */ NULL);
    }
    return TRUE;
}

std::string PollHandle(const HANDLE handle) {
    DWORD amount_read;
    CHAR buffer[BUFFER_SIZE + 1];  // Ensure space for null-terminator.
    BOOL success = FALSE;
    std::ostringstream str;

    // Spin until we are unable to read the pipe from child process anymore.
    for (;;) {
        success = ReadFile(
            /* hFile= */ handle,
            /* lpBuffer= */ buffer,
            /* nNumberOfBytesToRead= */ BUFFER_SIZE,
            /* lpNumberOfBytesRead= */ &amount_read,
            /* lpOverlapped= */ NULL);

        if (!success || amount_read == 0) {
            break;
        }

        // ensure null termination.
        buffer[amount_read] = '\0';
        str << buffer;
    }

    return str.str();
}

}  // namespace

struct SubprocessExecutor::PlatformDependentFields {
    HANDLE hStdOutPipeRead = NULL;
    HANDLE hStdOutPipeWrite = NULL;
    HANDLE hStdErrPipeRead = NULL;
    HANDLE hStdErrPipeWrite = NULL;
    HANDLE hProcess = NULL;
    DWORD dwProcessId = 0;
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
    if (is_running_ && fields->hProcess) {
        // Force kill other process.
        // No throw and no block so it's "safe" to call in dtor.
        TerminateProcess(fields->hProcess, /* uExitCode= */ 1);
        // Dtor of future will block until both threads terminate.
        // Since we have killed the other process, this should terminate
        // eventually.
        fields->captured_output.reset();
        fields->captured_error.reset();

        CleanupHandle(fields->hStdOutPipeRead);
        CleanupHandle(fields->hStdOutPipeWrite);
        CleanupHandle(fields->hStdErrPipeRead);
        CleanupHandle(fields->hStdErrPipeWrite);
        CleanupHandle(fields->hProcess);
        fields->dwProcessId = 0;
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

    SECURITY_ATTRIBUTES security_attributes;
    security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attributes.bInheritHandle = TRUE;
    security_attributes.lpSecurityDescriptor = NULL;

    if (capture_output_) {
        if (!CreatePipe(&fields->hStdOutPipeRead, &fields->hStdOutPipeWrite,
                        &security_attributes, 0)) {
            throw std::runtime_error(
                "Unable to create stdout pipe while running: " + command_);
        }
        // Do not let child process inherit the read handles (they can only
        // write).
        if (!SetHandleInformation(fields->hStdOutPipeRead, HANDLE_FLAG_INHERIT,
                                  0)) {
            throw std::runtime_error(
                "Unable to set stdout handle info while running: " + command_);
        }
        if (!CreatePipe(&fields->hStdErrPipeRead, &fields->hStdErrPipeWrite,
                        &security_attributes, 0)) {
            throw std::runtime_error(
                "Unable to create err pipe while running: " + command_);
        }
        if (!SetHandleInformation(fields->hStdErrPipeRead, HANDLE_FLAG_INHERIT,
                                  0)) {
            throw std::runtime_error(
                "Unable to set err handle info while running: " + command_);
        }
    }

    PROCESS_INFORMATION proc_info;
    ZeroMemory(&proc_info, sizeof(PROCESS_INFORMATION));

    STARTUPINFOW start_info;
    ZeroMemory(&start_info, sizeof(STARTUPINFOW));
    start_info.cb = sizeof(STARTUPINFOW);
    start_info.hStdError = fields->hStdErrPipeWrite;
    start_info.hStdOutput = fields->hStdOutPipeWrite;
    start_info.dwFlags |= STARTF_USESTDHANDLES;

    std::wstring command = ConvertToWString(command_);
    BOOL success = CreateProcessW(
        /* lpApplicationName= */ NULL,
        /* lpCommandLine= */ &command[0],
        /* lpProcessAttributes= */ NULL,
        /* lpThreadAttributes= */ NULL,
        /* bInheritHandles= */ TRUE,
        /* dwCreationFlags= */ 0,
        /* lpEnvironment= */ NULL,
        /* lpCurrentDirectory= */ NULL,
        /* lpStartupInfo= */ &start_info,
        /* lpProcessInformation= */ &proc_info);

    if (!success) {
        throw std::runtime_error("Unable to create process to run: " +
                                 command_);
    }
    is_running_ = true;
    fields->hProcess = proc_info.hProcess;
    fields->dwProcessId = proc_info.dwProcessId;

    // Close handles to child's primary thread. Not needed in this context.
    CleanupHandle(proc_info.hThread);

    // IMPORTANT! Close handles to stdout and stderr write.
    // Since we will not use these we must close them.
    // Otherwise we get a deadlock since we hold the write pipe, but child needs
    // to write too!
    // https://devblogs.microsoft.com/oldnewthing/20110707-00/?p=10223
    CleanupHandle(fields->hStdOutPipeWrite);
    CleanupHandle(fields->hStdErrPipeWrite);

    if (capture_output_) {
        // Launch 2 theads to read from stdout and stderr respectively.
        fields->captured_output = std::make_unique<std::future<std::string>>(
            std::async(std::launch::async,
                       [this] { return PollHandle(fields->hStdOutPipeRead); }));
        fields->captured_error = std::make_unique<std::future<std::string>>(
            std::async(std::launch::async,
                       [this] { return PollHandle(fields->hStdErrPipeRead); }));
    }
}

SubprocessExecutor::Output SubprocessExecutor::WaitUntilFinished(
    std::optional<int> timeout_ms) {
    if (!is_running_) {
        throw std::runtime_error(
            "You must call Start() before you are able to wait.");
    }

    // First wait to see if it finishes in time.
    if (timeout_ms &&
        WaitForSingleObject(fields->hProcess, *timeout_ms) == WAIT_TIMEOUT) {
        // If not then ask it nicely to close.
        EnumWindows(&SendWMCloseMsg, fields->dwProcessId);
        // Wait for another timeout_ms before we force terminate.
        if (WaitForSingleObject(fields->hProcess, *timeout_ms) ==
            WAIT_TIMEOUT) {
            // If still not finished, then force kill.
            TerminateProcess(fields->hProcess, /* uExitCode= */ 0);
        }
    }

    SubprocessExecutor::Output output;
    if (fields->captured_output) {
        // Block until stdout thread finishes.
        output.subproc_stdout = fields->captured_output->get();
    }
    if (fields->captured_error) {
        // Block until stderr thread finishes.
        output.subproc_stderr = fields->captured_error->get();
    }

    is_running_ = false;

    // Cleanup all fields as a safety measure.
    CleanupHandle(fields->hStdOutPipeRead);
    CleanupHandle(fields->hStdOutPipeWrite);
    CleanupHandle(fields->hStdErrPipeRead);
    CleanupHandle(fields->hStdErrPipeWrite);
    CleanupHandle(fields->hProcess);
    fields->dwProcessId = 0;
    fields->captured_output.reset();
    fields->captured_error.reset();

    return output;
}

}  // namespace subprocess
}  // namespace subtitler
