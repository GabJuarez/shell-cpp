#include <unistd.h>
#include <iostream>

namespace sh::builtins {
    void pwd() {
        size_t size = 1024;
        char buffer[size];
        getcwd(buffer, size);

        if (getcwd(buffer, size) != nullptr) {
            std::cout << buffer << std::endl;
            return;
        }
        std::cout << "pwd: error retrieving current directory" << std::endl;
    }
}