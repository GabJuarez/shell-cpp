#pragma once
#include <string>
#include <vector>

namespace sh::builtins {
    // List of built-in commands
    const std::string builtins[] = {"echo", "exit", "type", "pwd", "cd", "history"};

    // Exposing builtins namespace functions
    void echo(const std::vector<std::string> &args);

    void exit1();

    void type(const std::vector<std::string> &args);

    void pwd();

    void cd(const std::vector<std::string> &args);

    // history builtin: supports at least `history -a <path>`
    void history(const std::vector<std::string> &args);

    void history_init();

    // inline function to check if a command is a built-in
    inline bool is_builtin(const std::string &command) {
        for (const std::string &builtin: sh::builtins::builtins) {
            if (command == builtin) {
                return true;
            }
        }
        return false;
    }
} // namespace sh::builtins
