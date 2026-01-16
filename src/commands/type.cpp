#include <iostream>
#include <string>
#include <vector>
using namespace std;

void type(vector<string> args) {
  // List of available commands
  string commands[] = {"echo", "exit", "type"};

  if (args.empty()) {
    cout << endl;
    return;
  }

  for (const string arg : args) {
    bool found = false;
    for (int i = 0; i < commands->size(); i++) {
      if (arg == commands[i]) {
        found = true;
        cout << arg + " is a shell builtin" << endl;
        break;
      }
    }
    if (!found) {
      cout << arg + ": command not found" << endl;
    }
  }
}
