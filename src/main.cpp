#include "commands/builtins.hpp"
#include "utils/exec.hpp"
#include "helpers/helpers.hpp"
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Map wiht optional arguments
  std::unordered_map<std::string, std::function<void(std::vector<std::string>)>>
      commands;

  // Mapping the commands so we can use the values to call the functions
  // Using lambdas to adapt the function signatures
  commands["echo"] = [](const std::vector<std::string> &args) { sh::builtins::echo(args); };
  commands["type"] = [](const std::vector<std::string> &args) { sh::builtins::type(args); };
  commands["exit"] = [](const std::vector<std::string> &args) { sh::builtins::exit1(); };
  commands["pwd"] = [](const std::vector<std::string> &args){ sh::builtins::pwd(); };
  commands["cd"] = [](const std::vector<std::string> &args){ sh::builtins::cd(args); };

  while (true) {
    // Display prompt
    std::cout << "$ " << std::flush;

    // Read user input
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
      continue;
    }

    // removing the spaces from the beginning and end of the string
    input = helpers::trim(input);

    // Vector to hold command and arguments
    std::vector<std::string> args;
    std::string command;
    std::stringstream ss(input);
    ss >> command;

    // Removing the command from the stringstream and cleaning side spaces
    std::string r_args = std::stringstream(helpers::trim(ss.str().substr(command.length()))).str();
    args = helpers::split_args(r_args);

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
      continue;

    } catch ([[maybe_unused]] std::exception &e) {
      // If the command doesn't exist an error message will be printed
      std::cout << input + ": command not found" << std::endl;
    }
  }
}
