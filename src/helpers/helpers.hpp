#pragma once
#include <string>

namespace helpers {
    std::string trim(const std::string &str);
    std::vector<std::string> split_args(const std::string &r_args);
}