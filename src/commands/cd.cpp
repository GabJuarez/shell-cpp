#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

namespace sh::builtins {
    void cd(const std::vector<std::string> &args){
        const std::filesystem::path current = std::filesystem::current_path();
        std::filesystem::path target_path = args[0];

        if (args.size() > 1) {
            std::cout << "cd: too many arguments" << std::endl;
            return;
        }

        try {
            if (target_path.empty()) {

            }else if (args.size() > 1 && *args[0].begin() == '.') {
                target_path = std::filesystem::path(current.string() + "/" + args[0].substr(2, args[0].length()));
                std::filesystem::current_path(std::filesystem::path(target_path));
            }
            else {
                std::filesystem::current_path(std::filesystem::path(args[0]));
            }

        }catch (std::exception &e) {
            std::cout << "cd: " + args[0] + ": No such file or directory" << std::endl;
        }


    }
}
