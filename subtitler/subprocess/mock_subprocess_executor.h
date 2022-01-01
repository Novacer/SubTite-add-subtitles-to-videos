#ifndef SUBTITLER_SUBPROCESS_MOCK_SUBPROCESS_EXECUTOR_H
#define SUBTITLER_SUBPROCESS_MOCK_SUBPROCESS_EXECUTOR_H

#include <gmock/gmock.h>
#include "subtitler/subprocess/subprocess_executor.h"

namespace subtitler {
namespace subprocess {

/**
 * Mock class used to test SubprocessExecutor
 */
class MockSubprocessExecutor : public SubprocessExecutor {
public:
    MOCK_METHOD(void, SetCommand, (const std::string &), (override));
    MOCK_METHOD(void, CaptureOutput, (bool), (override));
    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(SubprocessExecutor::Output, WaitUntilFinished, (std::optional<int>), (override));
};

} // namespace subtitler
} // namespace subprocess

#endif
