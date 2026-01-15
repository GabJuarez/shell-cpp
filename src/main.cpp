#include <cstdlib>
#include <iostream>
#include <string>
using namespace std;

int main() {
  // Flush after every std::cout / std:cerr
  cout << unitbuf;
  cerr << unitbuf;

  while (true) {
    // Display prompt
    cout << "$ ";

    // Read user input
    string input;
    getline(cin, input);

    // If input is empty, continue to next iteration
    if (input.empty())
      continue;

    // 'exit' command hardcoded to terminate the program
    if (input == "exit") {
      break;
      return 0;
    }

    // Just printing an error message for future command handling
    cout << input + ": command not found" << endl;
  }
}
