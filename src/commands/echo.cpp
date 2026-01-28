#include <iostream>
#include <string>
#include <vector>

namespace sh::builtins {
    void echo(const std::vector<std::string> &args) {
        bool first = true;
        for (const auto &arg: args) {
            if (arg == "<newline>") {
                std::cout << std::endl;
                first = true;
                continue;
            }
            if (!first) std::cout << ' ';
            std::cout << arg;
            first = false;
        }
        std::cout << std::endl;
    }
} // namespace sh::builtins
