#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace sh::exec {
    bool is_executable_cu(const fs::path &p);

    void exec_command(const std::string &command, const std::vector<std::string> &args);

    // Execute a pipeline of two external commands: left | right
    // Each vector contains command name followed by its args
    void exec_pipeline(const std::vector<std::string> &left_cmd,
                       const std::vector<std::string> &right_cmd);

    // Execute a pipeline of n commands
    // Each inner vector contains command name followed by its arguments
    void exec_pipeline_chain(const std::vector<std::vector<std::string> > &commands);
}
