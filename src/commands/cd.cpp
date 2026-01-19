#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <pwd.h>
namespace sh::builtins {
    void cd(const std::vector<std::string> &args){
        const std::filesystem::path current = std::filesystem::current_path();

        if (args.size() > 1) {
            std::cout << "cd: string not in pwd: " << args[0] << std::endl;
            return;
        }

        try {
            if (args.empty() == true || args[0] == "~") {
                const char* home;

                if (getenv("HOME") == nullptr) {
                    uid_t uid = getuid();
                    struct passwd *pw = getpwuid(uid);
                    home = pw->pw_dir;
                }else {
                    home = getenv("HOME");
                }

                if (home != nullptr) {
                    std::filesystem::current_path(std::filesystem::path(home));
                } else {
                    std::cout << "cd: HOME not set" << std::endl;
                }

            }else if (args.size() > 1 && *args[0].begin() == '.') {
                std::filesystem::path target_path = args[0];
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
