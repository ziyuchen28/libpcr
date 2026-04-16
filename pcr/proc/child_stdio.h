#pragma once

#include <optional>

namespace pcr::proc {

// if a field is std::nullopt, the child inherits the parent's current stdio fd
// if a field has a value, that fd will be dup2()'d onto child stdin/stdout/stderr
struct ChildStdioMap 
{
    std::optional<int> stdin_fd;
    std::optional<int> stdout_fd;
    std::optional<int> stderr_fd;
};

} // namespace pcr::proc
