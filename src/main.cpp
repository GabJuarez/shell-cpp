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
#include <fstream>

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

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

    while (true) {
        // Display prompt
        std::cout << "$ " << std::flush;

        // Read user input
        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) {
            continue;
        }

        // Removing the spaces from the beginning and end of the string
        input = helpers::trim(input);

        // If it begins with quotes it'll try to execute the file
        if (input[0] == '\'' || input[0] == '\"') {
            // Splitting the entire vector
            std::vector<std::string> exec = helpers::parser(input);

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

        if (r_args.find('>') == r_args.npos) {
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
                bool found = false;
                std::vector<std::string> b_operator;
                std::string file_name;

                for (auto &s: args) {
                    if (s == ">") {
                        found = true;
                        continue;
                    }
                    if (!found) {
                        b_operator.push_back(s);
                        continue;
                    }
                    file_name = s;
                }

                // The file that we're redirecting the output to
                std::ofstream outfile;
                outfile.open(file_name);

                // Store default cout's buffer
                std::streambuf *cout_buf = std::cout.rdbuf();

                // Get the streambuf of the file
                std::streambuf *file_buf = outfile.rdbuf();

                // Redirect the o buffer
                std::cout.rdbuf(file_buf);

                if (sh::builtins::is_builtin(command)) {
                    // Calling the funct with the correct map depending on the args number
                    if (args.empty()) {
                        commands[command]({});
                        std::cout.rdbuf(cout_buf);
                        continue;
                    }
                    commands[command](b_operator);
                    std::cout.rdbuf(cout_buf);
                    continue;
                }

                sh::exec::exec_command(command, b_operator);
                std::cout.rdbuf(cout_buf);
            } catch ([[maybe_unused]] std::exception &e) {
                std::cout << input + ": command not found" << std::endl;
            }
        }
    }
}
