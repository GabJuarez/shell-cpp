#include <iostream>
#include <string>
#include <vector>

namespace sh::builtins {
    void echo(const std::vector<std::string> &args) {
        for (const auto &arg: args) {
            if (arg == "<newline>") {
                std::cout << std::endl;
                continue;
            }
            std::cout << arg << " ";
        }
        std::cout << std::endl;
    }
} // namespace sh::builtins
