#include "commands/builtins.hpp"
#include "helpers/helpers.hpp"
#include "utils/exec.hpp"
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "completion/completion.hpp"
#include "parser/parser.hpp"
#include <cstdlib>

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // Initialize readline completion for builtins
    completion::initialize();

    // History persistence configuration
    // Prefer HISTFILE (standard) -> fallback to SHELL_HISTORY_FILE -> default
    const char *hist_env_std = std::getenv("HISTFILE");
    const char *hist_env_alt = std::getenv("SHELL_HISTORY_FILE");
    std::string history_file;
    if (hist_env_std && hist_env_std[0] != '\0') {
        history_file = hist_env_std;
    } else if (hist_env_alt && hist_env_alt[0] != '\0') {
        history_file = hist_env_alt;
    } else {
        const char *home = std::getenv("HOME");
        history_file = (home ? std::string(home) : std::string(".")) + "/.shell_history";
    }

    bool do_read = true;
    bool do_write = true;
    const char *hr = std::getenv("SHELL_HISTORY_READ");
    if (hr && std::string(hr) == "0") do_read = false;
    const char *hw = std::getenv("SHELL_HISTORY_WRITE");
    if (hw && std::string(hw) == "0") do_write = false;

    if (do_read) {
        // Attempt to read history from file (no failure if file missing)
        read_history(history_file.c_str());
    }

    // Initialize history module internal state
    sh::builtins::history_init();

    // Map wiht optional arguments
    std::unordered_map<std::string, std::function<void(std::vector<std::string>)> >
            commands;

    // Mapping the commands so we can use the values to call the functions
    // Using lambdas to adapt the function signatures
    commands["echo"] = [](const std::vector<std::string> &args) {
        sh::builtins::echo(args);
    };
    commands["type"] = [](const std::vector<std::string> &args) {
        sh::builtins::type(args);
    };
    commands["exit"] = [](const std::vector<std::string> &args) {
        sh::builtins::exit1();
    };
    commands["pwd"] = [](const std::vector<std::string> &args) {
        sh::builtins::pwd();
    };
    commands["cd"] = [](const std::vector<std::string> &args) {
        sh::builtins::cd(args);
    };
    commands["history"] = [](const std::vector<std::string> &args) {
        sh::builtins::history(args);
    };

    while (true) {
        // Display prompt and read input using readline so TAB completion works
        char *raw_line = readline("$ ");
        if (!raw_line) {
            // EOF (Ctrl+D) -> exit shell
            break;
        }

        std::string input(raw_line);
        // readline allocates with malloc
        std::free(raw_line);

        if (input.empty()) {
            continue;
        }

        // Removing the spaces from the beginning and end of the string
        input = helpers::trim(input);

        if (input.empty()) {
            continue;
        }

        // Add non-empty lines to history
        add_history(input.c_str());

        // If it begins with quotes it'll try to execute the file
        if (input[0] == '\'' || input[0] == '"') {
            // Splitting the entire vector
            std::vector<std::string> exec = sh::parser::parse(input);

            // Keeping the name of the executable in a separated variable
            const std::string file = exec[0];

            // Erasing the file's name from the args vector
            exec.erase(exec.begin());

            // If everything's okay then we execute the file
            sh::exec::exec_command(file, exec);
            continue;
        }

        // If not quotes
        // Vector to hold command and arguments
        std::vector<std::string> args;
        std::string command;
        std::stringstream ss(input);
        ss >> command;

        // Removing the command from the stringstream and cleaning side spaces
        std::string r_args =
                std::stringstream(helpers::trim(ss.str().substr(command.length())))
                .str();
        args = helpers::parser(r_args);

        // Handle pipelines (one or multiple '|' operators)
        {
            // Parse entire input into tokens and split on every '|' token.
            std::vector<std::string> all_tokens = sh::parser::parse(input);
            std::vector<std::vector<std::string> > cmds;
            std::vector<std::string> cur;

            for (const auto &t: all_tokens) {
                if (t == "|") {
                    if (!cur.empty()) {
                        cmds.push_back(cur);
                        cur.clear();
                    } else {
                        /* empty stage -> ignore */
                    }
                } else {
                    cur.push_back(t);
                }
            }
            if (!cur.empty()) cmds.push_back(cur);

            if (cmds.size() >= 2) {
                // Execute chain of N commands
                sh::exec::exec_pipeline_chain(cmds);
                continue;
            }
        }

        if (r_args.find('>') == std::string::npos && r_args.find("2>") == std::string::npos) {
            try {
                if (sh::builtins::is_builtin(command)) {
                    // Calling the funct with the correct map depending on the args number
                    if (args.empty()) {
                        commands[command]({});
                        continue;
                    }
                    commands[command](args);
                    continue;
                }

                sh::exec::exec_command(command, args);
            } catch ([[maybe_unused]] std::exception &e) {
                // If the command doesn't exist an error message will be printed
                std::cout << input + ": command not found" << std::endl;
            }
        } else {
            try {
                bool rd_stdout = false;
                bool rd_stderr = false;
                bool app_stdout = false;
                bool app_stderr = false;

                std::vector<std::string> b_operator;
                const char *file_name;

                for (auto &s: args) {
                    if (s == ">" || s == "1>") {
                        rd_stdout = true;
                        continue;
                    }
                    if (s == "2>") {
                        rd_stderr = true;
                        continue;
                    }

                    if (s == ">>" || s == "1>>") {
                        app_stdout = true;
                        continue;
                    }

                    if (s == "2>>") {
                        app_stderr = true;
                        continue;
                    }
                    if (!rd_stdout && !rd_stderr && !app_stdout && !app_stderr) {
                        b_operator.push_back(s);
                        continue;
                    }
                    file_name = s.c_str();
                }

                // Store defaults fds
                int old_stdout = dup(1);
                int old_stderr = dup(2);

                if (rd_stdout) {
                    // getting file's fd
                    if (int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644); fd != -1) {
                        // Redirect the stdout to that fd
                        dup2(fd, 1);
                    }
                }

                if (rd_stderr) {
                    // getting file's fd
                    if (int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644); fd != -1) {
                        // Redirect the stdout to that fd
                        dup2(fd, 2);
                    }
                }

                // Append instead of truncate the file
                if (app_stdout) {
                    // getting file's fd
                    if (int fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644); fd != -1) {
                        // Redirect the stdout to that fd
                        dup2(fd, 1);
                    }
                }

                if (app_stderr) {
                    if (int fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644); fd != -1) {
                        dup2(fd, 2);
                    }
                }

                if (sh::builtins::is_builtin(command)) {
                    if (args.empty()) {
                        commands[command]({});
                        // Reestoring its default fds
                        if (rd_stdout || app_stdout) dup2(old_stdout, 1);
                        if (rd_stderr || app_stderr) dup2(old_stderr, 2);
                    }
                    commands[command](b_operator);
                    // Reestoring its default fds
                    if (rd_stdout || app_stdout) dup2(old_stdout, 1);
                    if (rd_stderr || app_stderr) dup2(old_stderr, 2);
                    continue;
                }
                // If it's not a builtin, try to execute the bin
                sh::exec::exec_command(command, b_operator);

                // Reestoring its default fds
                if (rd_stdout || app_stdout) dup2(old_stdout, 1);
                if (rd_stderr || app_stderr) dup2(old_stderr, 2);
            } catch ([[maybe_unused]] std::exception &e) {
                std::cout << input + ": command not found" << std::endl;
            }
        }
    }

    // Write history to file on exit (no failure if file not writable)
    if (do_write) {
        write_history(history_file.c_str());
    }
}
