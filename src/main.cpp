// #include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Forward declarations for commands and helpers implemented in other
// translation units
// if the project gets bigger these should be moved to their respective header
// files

//with args
void echo(vector<string> args);
void type(vector<string> args);

//no args
void exit1();

//helpers
string trim(const string &str);


int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  // Mapping the commands so we can use the values to call the functions
  map<string, function<void(vector<string>)>> commands_with_args;
  commands_with_args["echo"] = echo;
  commands_with_args["type"] = type;

  map<string, function<void()>> commands_no_args;
  commands_no_args["exit"] = exit1;

  while (true) {
    // Display prompt
    cout << "$ ";

    // Read user input
    string input;
    getline(cin, input);

    if (input.empty()) {
      continue;
    }

    // removing the spaces from the beginning and end of the string
    input = trim(input);

    // Vector to hold command and arguments
    vector<string> args;
    string command;
    stringstream ss(input);
    ss >> command;

    // Removing the command from the stringstream
    ss = stringstream(ss.str().substr(command.length()));

    // cleaning the spaces at the beginning after removing the first
    // word(command)
    ss = stringstream(trim(ss.str()));

    if (!ss.str().empty())
      while (ss >> input) {
        args.push_back(input);
      }

    try {
      // Calling the funct with the correct map depending on the args number
      if (args.empty()) {
        commands_no_args[command]();
      }
      commands_with_args[command](args);
    } catch (exception) {
      // If the command doesn't exist a error message will be printed
      cout << input + ": command not found" << endl;
    }
  }
}
