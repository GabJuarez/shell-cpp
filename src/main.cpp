// #include <boost/algorithm/string.hpp>
#include "utils/builtins.hpp"
#include "utils/exec.hpp"
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations for commands and helpers implemented in other
// translation units
// if the project gets bigger these should be moved to their respective header
// files

// Commands
void echo(std::vector<std::string> args);
void type(std::vector<std::string> args);
void exit1();

// helpers
std::string trim(const std::string &str);

int main() {
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // Map wiht optional arguments
  std::unordered_map<std::string, std::function<void(std::vector<std::string>)>>
      commands;

  // Mapping the commands so we can use the values to call the functions
  // Using lambdas to adapt the function signatures
  commands["echo"] = [](std::vector<std::string> args) { echo(args); };
  commands["type"] = [](std::vector<std::string> args) { type(args); };
  commands["exit"] = [](std::vector<std::string> args) { exit1(); };

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
    input = trim(input);

    // Vector to hold command and arguments
    std::vector<std::string> args;
    std::string command;
    std::stringstream ss(input);
    ss >> command;

    // Removing the command from the stringstream
    ss = std::stringstream(ss.str().substr(command.length()));
    // cleaning the spaces at the beginning after removing the first
    // word(command)
    ss = std::stringstream(trim(ss.str()));

    if (!ss.str().empty())
      while (ss >> input) {
        args.push_back(input);
      }

    try {
      if (sh::builtins::is_builtin(command)) {
        // Calling the funct with the correct map depending on the args number
        if (args.empty()) {
          commands[command]({});
        }
        commands[command](args);
      }

      sh::exec::exec_command(command, args);
      continue;

    } catch (std::exception) {
      // If the command doesn't exist a error message will be printed
      std::cout << input + ": command not found" << std::endl;
    }
  }
}
