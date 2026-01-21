#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace sh::exec {
bool is_executable_cu(const fs::path &p);
void exec_command(const std::string &command, const std::vector<std::string> &args);
} // namespace sh::exec