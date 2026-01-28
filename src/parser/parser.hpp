#pragma once

#include <string>
#include <vector>

namespace sh::parser {
    // Parse commands into tokens
    std::vector<std::string> parse(const std::string &r_args);
}
