#include "subtitler/subprocess/subprocess_executor.h"

#include <windows.h>
#include <stdexcept>
#include <sstream>

namespace subtitler {
namespace subprocess {

namespace {

const int BUFFER_SIZE = 4096;

std::wstring ConvertStringToWString(const std::string& str) {
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

} // namespace

struct SubprocessExecutor::PlatformDependentFields{
    HANDLE hStdInPipeRead = NULL;
    HANDLE hStdInPipeWrite = NULL;
    HANDLE hStdOutPipeRead = NULL;
    HANDLE hStdOutPipeWrite = NULL;
};

SubprocessExecutor::SubprocessExecutor() :
    command_{},
    capture_stdout_{false},
    fields{std::make_unique<PlatformDependentFields>()} {}

SubprocessExecutor::~SubprocessExecutor() = default;

void SubprocessExecutor::SetCommand(const std::string& command) {
    command_ = command;
}

void SubprocessExecutor::CaptureStdout(bool capture) {
    capture_stdout_ = capture;
}

void SubprocessExecutor::Start() {
    SECURITY_ATTRIBUTES security_attributes;
    security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_attributes.bInheritHandle = TRUE;
    security_attributes.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&fields->hStdOutPipeRead, &fields->hStdOutPipeWrite, &security_attributes, 0)) {
        throw std::runtime_error("Unable to create stdout pipe while running: " + command_);
    }
    if (!SetHandleInformation(fields->hStdOutPipeRead, HANDLE_FLAG_INHERIT, 0)) {
        throw std::runtime_error("Unable to set stdout handle info while running: " + command_);
    }
    if (!CreatePipe(&fields->hStdInPipeRead, &fields->hStdInPipeWrite, &security_attributes, 0)) {
        throw std::runtime_error("Unable to create stdin pipe while running: " + command_);
    }
    if (!SetHandleInformation(fields->hStdInPipeWrite, HANDLE_FLAG_INHERIT, 0)) {
        throw std::runtime_error("Unable to set stdin handle info while running: " + command_);
    }

    PROCESS_INFORMATION proc_info; 
    ZeroMemory(&proc_info, sizeof(PROCESS_INFORMATION));
    
    STARTUPINFOW start_info;
    ZeroMemory(&start_info, sizeof(STARTUPINFOW));
    start_info.cb = sizeof(STARTUPINFOW);
    start_info.hStdError = fields->hStdOutPipeWrite;
    start_info.hStdOutput = fields->hStdOutPipeWrite;
    start_info.hStdInput = fields->hStdInPipeRead;
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

    // Close handles to child process and primary thread.
    // We can use other methods to monitor status.
    CloseHandle(proc_info.hProcess);
    CloseHandle(proc_info.hThread);

    // IMPORTANT! Close handles to stdout write and stdin read.
    // Since we will not use these we must close them.
    // Otherwise we get a deadlock waiting for stdout!
    CloseHandle(fields->hStdOutPipeWrite);
    CloseHandle(fields->hStdInPipeRead);
}

std::string SubprocessExecutor::WaitUntilFinished() {
    DWORD amount_read;
    CHAR buffer[BUFFER_SIZE];
    BOOL success = FALSE;
    std::ostringstream str;
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

        if (capture_stdout_) {
            str << buffer;
        }
    }
    return str.str();
}

} // namespace subprocess
} // namespace subtitler
