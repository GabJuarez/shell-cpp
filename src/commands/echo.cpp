#include <iostream>
#include <string>
#include <vector>

namespace sh::builtins {
void echo(const std::vector<std::string> &args) {
  for (const auto &arg : args) {
    std::cout << arg << " ";
  }
  std::cout << std::endl;
}
} // namespace sh::builtins