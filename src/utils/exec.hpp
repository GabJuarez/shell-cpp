#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace sh::exec{
    bool is_executable_cu(const fs::path &p);
    void exec_command(std::string &command, std::vector<std::string> &args);
}