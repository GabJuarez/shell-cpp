#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/_types/_gid_t.h>
#include <sys/_types/_off_t.h>
#include <sys/_types/_s_ifmt.h>
#include <sys/_types/_uid_t.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace std;
namespace fs = filesystem;

bool is_executable_cu(const fs::path &p) {
  // Alias so I don't have to rewrite
  fs::perms perms = fs::status(p).permissions();

  // struct to save the file's info
  struct stat st;
  if (stat(p.c_str(), &st) != 0) {
    cout << "Failed to get status of the path: " << string(p) << endl;
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

void type(vector<string> args) {
  // List of available builtins
  string builtins[] = {"echo", "exit", "type"};

  if (args.empty()) {
    cout << endl;
    return;
  }

  for (const string arg : args) {
    bool found = false;
    for (int i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++) {
      if (arg.compare(builtins[i]) == 0) {
        found = true;
        cout << arg + " is a shell builtin" << endl;
        break;
      }
    }
    if (!found) {
      // PATHS
      vector<fs::path> paths;
      string r_paths = getenv("PATH");

      // string stream to split the path string using ':' as a del
      stringstream ss(r_paths);
      string temp_str;
      char del = ':';

      while (getline(ss, temp_str, del)) {
        paths.push_back(fs::path(temp_str));
      }

      for (fs::path &path : paths) {
        try {
          for (const auto entry : fs::recursive_directory_iterator(path)) {
            if (fs::is_regular_file(entry)) {
              if (entry.path().stem() == arg &&
                  is_executable_cu(entry.path().c_str())) {
                cout << arg + " is " + path.string() << endl;
                found = true;
                break;
              }
            }
          }

        } catch (filesystem::filesystem_error &e) {
          // Ignore errors related to permissions or non-existing directories
        }

        // Break the paths foreach loop if found
        if (found) break;

      }
      if (!found) {
        cout << arg + ": not found" << endl;
      }
    }
  }
}
