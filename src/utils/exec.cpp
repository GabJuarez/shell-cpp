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

            pid_t pid = fork();
            if (pid == 0) {
                execvp(full_path_opt->c_str(), argv.data());
                perror("execvp");
                _exit(1);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }

    // Helper: execute a single command (builtin or external) in the current process
    // after stdin/stdout have been redirected appropriately.
    static void run_command_in_current_process(const std::vector<std::string> &cmd) {
        if (cmd.empty()) return;
        const std::string &name = cmd[0];
        std::vector<std::string> args(cmd.begin() + 1, cmd.end());

        if (sh::builtins::is_builtin(name)) {
            if (name == "echo") sh::builtins::echo(args);
            else if (name == "type") sh::builtins::type(args);
            else if (name == "pwd") sh::builtins::pwd();
            else if (name == "cd") sh::builtins::cd(args);
            else if (name == "exit") sh::builtins::exit1();
        } else {
            // find executable path
            std::optional<std::string> path = paths::get_first_entry(name);
            if (!path) {
                std::cout << name << ": command not found" << std::endl;
                return;
            }
            // Prepare argv
            std::vector<char *> argv;
            argv.push_back(const_cast<char *>(name.c_str()));
            for (size_t i = 1; i < cmd.size(); ++i) argv.push_back(const_cast<char *>(cmd[i].c_str()));
            argv.push_back(nullptr);
            execvp(path->c_str(), argv.data());
            perror("execvp");
        }
    }

    // Execute a pipeline of N commands: commands[0] | commands[1] | ... | commands[N-1]
    void exec_pipeline_chain(const std::vector<std::vector<std::string> > &commands) {
        if (commands.empty()) return;
        size_t n = commands.size();
        if (n == 1) {
            // Single command: run normally
            // If it's a builtin, run in current process; otherwise exec_command
            const auto &cmd = commands[0];
            if (cmd.empty()) return;
            if (sh::builtins::is_builtin(cmd[0])) {
                run_command_in_current_process(cmd);
            } else {
                std::vector<std::string> args(cmd.begin() + 1, cmd.end());
                exec_command(cmd[0], args);
            }
            return;
        }

        // Create pipes: we need n-1 pipes
        std::vector<std::array<int, 2> > pipes(n - 1);
        for (size_t i = 0; i < n - 1; ++i) {
            if (pipe(pipes[i].data()) == -1) {
                perror("pipe");
                // cleanup previously created pipes
                for (size_t j = 0; j < i; ++j) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                return;
            }
        }

        std::vector<pid_t> pids;
        pids.reserve(n);

        for (size_t i = 0; i < n; ++i) {
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                // kill previous children
                for (pid_t child: pids) kill(child, SIGKILL);
                for (auto &p: pipes) {
                    close(p[0]);
                    close(p[1]);
                }
                return;
            }

            if (pid == 0) {
                // Child process
                // If not first, set stdin to read end of previous pipe
                if (i > 0) {
                    if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                        perror("dup2");
                        _exit(1);
                    }
                }
                // If not last, set stdout to write end of current pipe
                if (i + 1 < n) {
                    if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                        perror("dup2");
                        _exit(1);
                    }
                }

                // Close all pipe fds in child
                for (auto &p: pipes) {
                    close(p[0]);
                    close(p[1]);
                }

                // Execute command (builtin or external)
                const auto &cmd = commands[i];
                if (cmd.empty()) _exit(0);

                if (sh::builtins::is_builtin(cmd[0])) {
                    // Run builtin in child process
                    std::vector<std::string> bargs(cmd.begin() + 1, cmd.end());
                    if (cmd[0] == "echo") sh::builtins::echo(bargs);
                    else if (cmd[0] == "type") sh::builtins::type(bargs);
                    else if (cmd[0] == "pwd") sh::builtins::pwd();
                    else if (cmd[0] == "cd") sh::builtins::cd(bargs);
                    else if (cmd[0] == "exit") sh::builtins::exit1();
                    _exit(0);
                } else {
                    // External command
                    std::optional<std::string> path = paths::get_first_entry(cmd[0]);
                    if (!path) {
                        std::cout << cmd[0] << ": command not found" << std::endl;
                        _exit(1);
                    }
                    if (!is_executable_cu(*path)) {
                        std::cout << cmd[0] << ": not executable" << std::endl;
                        _exit(1);
                    }
                    std::vector<char *> argv;
                    argv.push_back(const_cast<char *>(cmd[0].c_str()));
                    for (size_t k = 1; k < cmd.size(); ++k) argv.push_back(const_cast<char *>(cmd[k].c_str()));
                    argv.push_back(nullptr);
                    execvp(path->c_str(), argv.data());
                    perror("execvp");
                    _exit(1);
                }
            }

            // Parent: record child pid
            pids.push_back(pid);
        }

        // Parent: close all pipe fds
        for (auto &p: pipes) {
            close(p[0]);
            close(p[1]);
        }

        // Wait for all children
        int status;
        for (pid_t child: pids) {
            waitpid(child, &status, 0);
        }
    }

    void exec_pipeline(const std::vector<std::string> &left_cmd,
                       const std::vector<std::string> &right_cmd) {
        exec_pipeline_chain(std::vector<std::vector<std::string> >{left_cmd, right_cmd});
    }
} // namespace sh::exec
