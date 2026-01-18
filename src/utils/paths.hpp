#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace sh::paths {
    std::vector<std::string> get_all_entries(const std::filesystem::path &path);
    std::string get_first_entry(const std::string &command);
    std::vector<std::filesystem::path> get_paths();
}