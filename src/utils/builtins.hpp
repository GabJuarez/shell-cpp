#pragma once
#include <string>

namespace sh::builtins {
// List of built-in commands
const std::string builtins[] = {"echo", "exit", "type"};

// inline function to check if a command is a built-in
inline bool is_builtin(const std::string &command) {
  for (const std::string &builtin : sh::builtins::builtins) {
    if (command.compare(builtin) == 0) {
      return true;
    }
  }
  return false;
}
} 
