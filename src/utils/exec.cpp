#include "exec.hpp"
#include "../utils/paths.hpp"
#include "../commands/builtins.hpp"
#include <filesystem>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

namespace sh::exec {
    bool is_executable_cu(const fs::path &p) {
        // Alias so I don't have to rewrite
        fs::perms perms = fs::status(p).permissions();

        struct stat st{};
        if (stat(p.c_str(), &st) != 0) {
            std::cout << "not found" << std::endl;
        }

        uid_t uid = getuid();
        gid_t gid = getgid();

        uid_t fo = st.st_uid;
        gid_t fg = st.st_gid;

        if (uid == fo) {
            if ((fs::perms::owner_exec & perms) != fs::perms::none) {
                return true;
            }
        } else if (gid == fg) {
            if ((fs::perms::group_exec & perms) != fs::perms::none) {
                return true;
            }
        }

        if ((fs::perms::others_exec & perms) != fs::perms::none) {
            return true;
        }

        return false;
    }

    void exec_command(const std::string &command, const std::vector<std::string> &args) {
        std::optional<std::string> full_path_opt = paths::get_first_entry(command);

        if (full_path_opt == std::nullopt) {
            std::cout << command + ": command not found" << std::endl;
            return;
        }

        if (full_path_opt && is_executable_cu(*full_path_opt)) {
            if (full_path_opt->empty()) {
                execvp(command.c_str(), nullptr);
                return;
            }

            // Prepare arguments to posix function
            // only knows when the args end when it finds a nullptr
            std::vector<char *> argv;
            argv.push_back(const_cast<char *>(command.c_str()));

            for (auto &s: args) {
                argv.push_back(const_cast<char *>(s.c_str()));
            }
            argv.push_back(nullptr);

            // Forking the process
            // Fork duplicates the current process and creates a child and a parent
            // process
            pid_t pid = fork();
            if (pid == 0) {
                // Child: replace process image
                execvp(full_path_opt->c_str(), argv.data());
                perror("execvp");
                _exit(1);
            } else if (pid > 0) {
                // Parent: wait and return to the loop to print the prompt
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }

    void exec_pipeline(const std::vector<std::string> &left_cmd,
                       const std::vector<std::string> &right_cmd) {
        if (left_cmd.empty() || right_cmd.empty()) return;

        const std::string left_name = left_cmd[0];
        const std::string right_name = right_cmd[0];

        bool left_is_builtin = sh::builtins::is_builtin(left_name);
        bool right_is_builtin = sh::builtins::is_builtin(right_name);

        std::optional<std::string> left_path = std::nullopt;
        std::optional<std::string> right_path = std::nullopt;

        if (!left_is_builtin) left_path = paths::get_first_entry(left_name);
        if (!right_is_builtin) right_path = paths::get_first_entry(right_name);

        if (!left_is_builtin && !left_path) {
            std::cout << left_name << ": command not found" << std::endl;
            return;
        }
        if (!right_is_builtin && !right_path) {
            std::cout << right_name << ": command not found" << std::endl;
            return;
        }

        if (!left_is_builtin && left_path && !is_executable_cu(*left_path)) {
            std::cout << left_name << ": not executable" << std::endl;
            return;
        }
        if (!right_is_builtin && right_path && !is_executable_cu(*right_path)) {
            std::cout << right_name << ": not executable" << std::endl;
            return;
        }

        int pipefd[2];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            return;
        }

        pid_t left_pid = fork();
        if (left_pid == -1) {
            perror("fork");
            close(pipefd[0]);
            close(pipefd[1]);
            return;
        }

        if (left_pid == 0) {
            // Left child: write end -> stdout
            close(pipefd[0]);
            if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
                perror("dup2");
                _exit(1);
            }
            close(pipefd[1]);

            if (left_is_builtin) {
                // Prepare args (exclude command)
                std::vector<std::string> bargs;
                for (size_t i = 1; i < left_cmd.size(); ++i) bargs.push_back(left_cmd[i]);

                if (left_name == "echo") sh::builtins::echo(bargs);
                else if (left_name == "type") sh::builtins::type(bargs);
                else if (left_name == "pwd") sh::builtins::pwd();
                else if (left_name == "cd") sh::builtins::cd(bargs);
                else if (left_name == "exit") sh::builtins::exit1();
                _exit(0);
            } else {
                // External command
                std::vector<char *> argv_left;
                argv_left.push_back(const_cast<char *>(left_name.c_str()));
                for (size_t i = 1; i < left_cmd.size(); ++i)
                    argv_left.push_back(
                        const_cast<char *>(left_cmd[i].c_str()));
                argv_left.push_back(nullptr);

                execvp(left_path->c_str(), argv_left.data());
                perror("execvp");
                _exit(1);
            }
        }

        pid_t right_pid = fork();
        if (right_pid == -1) {
            perror("fork");
            kill(left_pid, SIGKILL);
            close(pipefd[0]);
            close(pipefd[1]);
            return;
        }

        if (right_pid == 0) {
            // Right child: read end -> stdin
            close(pipefd[1]);
            if (dup2(pipefd[0], STDIN_FILENO) == -1) {
                perror("dup2");
                _exit(1);
            }
            close(pipefd[0]);

            if (right_is_builtin) {
                std::vector<std::string> bargs;
                for (size_t i = 1; i < right_cmd.size(); ++i) bargs.push_back(right_cmd[i]);

                if (right_name == "echo") sh::builtins::echo(bargs);
                else if (right_name == "type") sh::builtins::type(bargs);
                else if (right_name == "pwd") sh::builtins::pwd();
                else if (right_name == "cd") sh::builtins::cd(bargs);
                else if (right_name == "exit") sh::builtins::exit1();
                _exit(0);
            } else {
                std::vector<char *> argv_right;
                argv_right.push_back(const_cast<char *>(right_name.c_str()));
                for (size_t i = 1; i < right_cmd.size(); ++i)
                    argv_right.push_back(
                        const_cast<char *>(right_cmd[i].c_str()));
                argv_right.push_back(nullptr);

                execvp(right_path->c_str(), argv_right.data());
                perror("execvp");
                _exit(1);
            }
        }

        // Parent: close pipe fds and wait for both children
        close(pipefd[0]);
        close(pipefd[1]);

        int status;
        waitpid(left_pid, &status, 0);
        waitpid(right_pid, &status, 0);
    }
} // namespace sh::exec
