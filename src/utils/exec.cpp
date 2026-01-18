#include "../utils/paths.hpp"
#include <filesystem>
#include <iostream>
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
  std::optional<std::string> full_path_opt = sh::paths::get_first_entry(command);
  if(full_path_opt == std::nullopt || full_path_opt->empty()) {
    return;
  }

  if (full_path_opt && sh::exec::is_executable_cu(*full_path_opt)) {
    if(full_path_opt->empty()) {
      execvp(command.c_str(), nullptr);
      return;
    }

    // Prepare arguments to posix function
    // only knows when the args end when it finds a nullptr
    std::vector<char *> argv;
    argv.push_back(const_cast<char *>(command.c_str()));

    for (auto &s : args) {
      argv.push_back(const_cast<char *>(s.c_str()));
    }
    argv.push_back(nullptr);

    // Forking the process
    // Fork duplicates the current process and creates a child and a parent process
    pid_t pid = fork();
    if (pid == 0) {
      // Child: replace process image
      execvp(full_path_opt->c_str(), argv.data());
      perror("execvp");
      _exit(1);
    } else if (pid > 0) {
      // Parent: wait and return to the loop to print the prompt
      int status;
      waitpid(pid, &status, 0);
    }
    return;
  }
}

} // namespace sh::exec
