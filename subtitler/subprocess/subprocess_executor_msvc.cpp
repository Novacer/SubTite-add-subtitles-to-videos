#include "subtitler/subprocess/subprocess_executor.h"

#pragma comment(lib, "User32.lib")

#include <windows.h>
#include <stdexcept>
#include <sstream>
#include <future>

namespace subtitler {
namespace subprocess {

namespace {

const int BUFFER_SIZE = 4096;
const int TIMEOUT_MS = 5000;

std::wstring ConvertStringToWString(const std::string &str) {
    int num_chars = MultiByteToWideChar(
        /* CodePage= */ CP_UTF8,
        /* dwFlags= */ MB_ERR_INVALID_CHARS,
        /* lpMultiByteStr= */ str.c_str(),
        /* cbMultiByte= */ str.length(),
        /* lpWideCharStr= */ NULL,
        /* cchWideChar= */ 0);

    if (num_chars <= 0) {
        return std::wstring();
    }

    std::wstring result;
    result.resize(num_chars);
    MultiByteToWideChar(
        /* CodePage= */ CP_UTF8,
        /* dwFlags= */ MB_ERR_INVALID_CHARS,
        /* lpMultiByteStr= */ str.c_str(),
        /* cbMultiByte= */ str.length(),
        /* lpWideCharStr= */ &result[0],
        /* cchWideChar= */ num_chars);
    return result;
}

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

} // namespace

struct SubprocessExecutor::PlatformDependentFields{
    HANDLE hStdOutPipeRead = NULL;
    HANDLE hStdOutPipeWrite = NULL;
    HANDLE hProcess = NULL;
    DWORD dwProcessId = 0;
    std::unique_ptr<std::future<std::string>> captured_output = nullptr;
};

SubprocessExecutor::SubprocessExecutor() :
    command_{},
    capture_output_{false},
    is_running_{false},
    fields{std::make_unique<PlatformDependentFields>()} {}

SubprocessExecutor::SubprocessExecutor(const std::string &command, bool capture_output) :
    command_{command},
    capture_output_{capture_output},
    is_running_{false},
    fields{std::make_unique<PlatformDependentFields>()} {}

SubprocessExecutor::~SubprocessExecutor() = default;

void SubprocessExecutor::SetCommand(const std::string& command) {
    command_ = command;
}

void SubprocessExecutor::CaptureOutput(bool capture) {
    capture_output_ = capture;
}

void SubprocessExecutor::Start() {
    if (is_running_) {
        throw std::runtime_error("You must call WaitUntilFinished() before starting again.");
    }

    SECURITY_ATTRIBUTES security_attributes;
    security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attributes.bInheritHandle = TRUE;
    security_attributes.lpSecurityDescriptor = NULL;

    if (capture_output_) {
        if (!CreatePipe(&fields->hStdOutPipeRead, &fields->hStdOutPipeWrite, &security_attributes, 0)) {
            throw std::runtime_error("Unable to create stdout pipe while running: " + command_);
        }
        if (!SetHandleInformation(fields->hStdOutPipeRead, HANDLE_FLAG_INHERIT, 0)) {
            throw std::runtime_error("Unable to set stdout handle info while running: " + command_);
        }
    }

    PROCESS_INFORMATION proc_info; 
    ZeroMemory(&proc_info, sizeof(PROCESS_INFORMATION));
    
    STARTUPINFOW start_info;
    ZeroMemory(&start_info, sizeof(STARTUPINFOW));
    start_info.cb = sizeof(STARTUPINFOW);
    start_info.hStdError = fields->hStdOutPipeWrite;
    start_info.hStdOutput = fields->hStdOutPipeWrite;
    start_info.dwFlags |= STARTF_USESTDHANDLES;

    std::wstring command = ConvertStringToWString(command_);
    BOOL success = CreateProcessW(
        /* lpApplicationName= */NULL,
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
        throw std::runtime_error("Unable to create process to run: " + command_);
    }
    is_running_ = true;
    fields->hProcess = proc_info.hProcess;
    fields->dwProcessId = proc_info.dwProcessId;

    // Close handles to child's primary thread. Not needed in this context.
    CleanupHandle(proc_info.hThread);

    // IMPORTANT! Close handles to stdout write.
    // Since we will not use these we must close them.
    // Otherwise we get a deadlock waiting for stdout!
    // https://devblogs.microsoft.com/oldnewthing/20110707-00/?p=10223
    CleanupHandle(fields->hStdOutPipeWrite);

    if (fields->hStdOutPipeWrite != NULL) {
        throw std::runtime_error("Why?");
    }

    if (capture_output_) {
        fields->captured_output = std::make_unique<std::future<std::string>>(
            std::async(std::launch::async,
            [&, this]{
                return SubprocessExecutor::PollChildProcessOutput(fields.get());
            })
        );
    }
}

std::string SubprocessExecutor::PollChildProcessOutput(const PlatformDependentFields *fields) {
    DWORD amount_read;
    CHAR buffer[BUFFER_SIZE + 1]; // Ensure space for null-terminator.
    BOOL success = FALSE;
    std::ostringstream str;
    
    // Spin until we are unable to read stdout of child process anymore. 
    for (;;) {
        success = ReadFile(
        /* hFile= */ fields->hStdOutPipeRead,
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

std::string SubprocessExecutor::WaitUntilFinished(std::optional<int> timeout_ms) {
    // First wait to see if it finishes in time.
    if (timeout_ms && WaitForSingleObject(fields->hProcess, *timeout_ms) == WAIT_TIMEOUT) {
        // If not then ask it nicely to close.
        EnumWindows(&SendWMCloseMsg, fields->dwProcessId);
        // Wait for another timeout_ms before we force terminate.
        if (WaitForSingleObject(fields->hProcess, *timeout_ms) == WAIT_TIMEOUT) {
            // If still not finished, then force kill.
            TerminateProcess(fields->hProcess, /* uExitCode= */ 0);
        }
    }

    std::string result;
    if (fields->captured_output) {
        // Block until thread finishes.
        result = fields->captured_output->get();
    }

    is_running_ = false;

    // Cleanup all fields as a safety measure.
    CleanupHandle(fields->hStdOutPipeRead);
    CleanupHandle(fields->hStdOutPipeWrite);
    CleanupHandle(fields->hProcess);
    fields->dwProcessId = 0;
    fields->captured_output.reset();

    return result;
}

} // namespace subprocess
} // namespace subtitler
