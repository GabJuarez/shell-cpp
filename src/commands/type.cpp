#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include "../utils/builtins.hpp"
#include "../utils/exec.hpp"
#include "../utils/paths.hpp"

namespace fs = std::filesystem;

void type(std::vector<std::string> args) {
  if (args.empty()) {
    std::cout << std::endl;
    return;
  }

  for (const std::string &arg : args) {
    bool found = false;
    for (const std::string &builtin : sh::builtins::builtins) {
      if (sh::builtins::is_builtin(arg)) {
        found = true;
        std::cout << arg + " is a shell builtin" << std::endl;
        break;
      }
      if (!found) {
        std::vector<fs::path> paths = sh::paths::get_paths();

        for (fs::path &path : paths) {
          try {
            for (const auto entry : fs::recursive_directory_iterator(path)) {
              if (fs::is_regular_file(entry)) {
                if (entry.path().stem() == arg &&
                    sh::exec::is_executable_cu(entry.path())) {
                  std::cout << arg + " is " + entry.path().string()
                            << std::endl;
                  found = true;
                  break;
                }
              }
            }

          } catch (std::filesystem::filesystem_error &e) {
            // Ignore errors related to permissions or non-existing
            // directories
          }

          // Break the paths foreach loop if found
          if (found)
            break;
        }
        if (!found) {
          std::cout << arg + ": not found" << std::endl;
          break;
        }
      }
    }
  }
}
