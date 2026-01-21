#pragma once
#include <string>
#include <vector>

namespace helpers {
    std::string trim(const std::string &str);

    std::vector<std::string> parser(const std::string &r_args);
} // namespace helpers
