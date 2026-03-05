#pragma once

#include <optional>
#include <string>
#include <vector>

namespace prism::proc {


struct EnvOverride 
{
    std::string key;
    std::string value;
};


struct ProcessSpec 
{
    std::string exe;

    std::vector<std::string> args;

    // child inherits parent env overrides
    std::vector<EnvOverride> env_overrides;

    // current working directory
    std::optional<std::string> cwd;
};

} // namespace prism::proc
