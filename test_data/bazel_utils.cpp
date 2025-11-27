#include "test_data/bazel_utils.h"

#include <memory>
#include <stdexcept>

#include "tools/cpp/runfiles/runfiles.h"

namespace test_data {

std::string GetBazelDataAbsolutePath(const std::string& relative_path) {
  using bazel::tools::cpp::runfiles::Runfiles;
  std::string error;

  std::unique_ptr<Runfiles> runfiles{Runfiles::CreateForTest(&error)};

  if (!runfiles) {
    throw std::runtime_error{"Could not create init runfiles: " + error};
  }

  return runfiles->Rlocation(relative_path);
}

}  // namespace test_data
