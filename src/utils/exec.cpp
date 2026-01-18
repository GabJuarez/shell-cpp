#include "../utils/paths.hpp"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

namespace sh::exec {
bool is_executable_cu(const fs::path &p) {
  // Alias so I don't have to rewrite
  fs::perms perms = fs::status(p).permissions();

  // struct to save the file's info
  struct stat st;
  if (stat(p.c_str(), &st) != 0) {
    std::cout << "not found" << std::endl;
  }

  // info of the current user
  uid_t uid = getuid();
  gid_t gid = getgid();

  // file's owner
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

void exec_command(std::string &command, std::vector<std::string> &args) {
  std::filesystem::path full_path = sh::paths::get_first_entry(command);
  if (sh::exec::is_executable_cu(full_path)) {
    if(full_path.string().empty()) {
      execvp(command.c_str(), nullptr);
      return;
    }
    std::string formatted_args;
    std::stringstream ss;
    for (const auto &arg : args) {
      ss << arg << " ";
    }
    formatted_args = ss.str();
    std::string exec_str = "." + full_path.string() + " " + formatted_args;
    system(exec_str.c_str());
  }
}

} // namespace sh::exec
