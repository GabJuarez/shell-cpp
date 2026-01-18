#include <iostream>
#include <string>
#include <vector>

void echo(std::vector<std::string> args) {
  for (int i = 0; i < args.size(); i++) {
    std::cout << args[i] << " ";
  }
  std::cout << std::endl;
}
