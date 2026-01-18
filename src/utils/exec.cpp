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

    // Prepare arguments to posix function
    // only knows when the args end when it finds a nullptr
    std::vector<char *> argv;

    for (auto &s : args) {
      argv.push_back(const_cast<char *>(s.c_str()));
    }

    std::string formatted_args;
    std::stringstream ss;

    // Including command so the argc is correct
    ss << command << " ";
    
    for (const auto &arg : argv) {
      std::string str = arg;
      ss << str << " ";
    }

    formatted_args = ss.str();
    

    //
    argv.push_back(nullptr);
    std::string exec_str =  full_path.string() + " " + formatted_args;
    execvp(full_path.c_str(), argv.data());
  }
}

} // namespace sh::exec
