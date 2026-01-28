#include <readline/readline.h>
#include <readline/history.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_set>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "../completion/completion.hpp"
#include "../commands/builtins.hpp"

namespace completion {
    // Generator called by readline to produce possible completions one by one
    static char *builtin_generator(const char *text, int state) {
        static std::vector<std::string> matches;
        static size_t match_index;
        static size_t len;

        if (state == 0) {
            matches.clear();
            match_index = 0;
            len = std::strlen(text);

            std::unordered_set<std::string> seen;

            // Builtins
            const size_t builtin_count = sizeof(sh::builtins::builtins) / sizeof(sh::builtins::builtins[0]);
            for (size_t i = 0; i < builtin_count; ++i) {
                const std::string &b = sh::builtins::builtins[i];
                if (b.compare(0, len, text) == 0) {
                    if (seen.insert(b).second) matches.push_back(b);
                }
            }

            // Executables in PATH
            const char *path_env = std::getenv("PATH");
            if (path_env) {
                std::string path_str(path_env);
                size_t start = 0;
                while (start <= path_str.size()) {
                    size_t pos = path_str.find(':', start);
                    std::string dir = (pos == std::string::npos)
                                          ? path_str.substr(start)
                                          : path_str.substr(start, pos - start);
                    if (dir.empty()) dir = "."; // empty PATH element means current directory

                    DIR *d = opendir(dir.c_str());
                    if (d) {
                        struct dirent *entry;
                        while ((entry = readdir(d)) != nullptr) {
                            const char *name = entry->d_name;
                            if (name[0] == '.') continue; // skip hidden entries
                            if (std::strncmp(name, text, len) != 0) continue;

                            // avoid duplicates
                            std::string name_str(name);
                            if (seen.find(name_str) != seen.end()) continue;

                            // Check executable permission
                            std::string fullpath = dir + "/" + name_str;
                            struct stat st;
                            if (stat(fullpath.c_str(), &st) == 0) {
                                if (S_ISREG(st.st_mode) && (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
                                    if (seen.insert(name_str).second) matches.push_back(name_str);
                                }
                            }
                        }
                        closedir(d);
                    }

                    if (pos == std::string::npos) break;
                    start = pos + 1;
                }
            }
        }

        // Return next match
        if (match_index < matches.size()) {
            const std::string &candidate = matches[match_index++];
            char *match = static_cast<char *>(std::malloc(candidate.size() + 1));
            if (!match) return nullptr;
            std::strcpy(match, candidate.c_str());
            return match;
        }

        // No more matches
        return nullptr;
    }

    // This function is called by readline to produce the matches for the current word
    static char **builtin_completion(const char *text, int start, int /*end*/) {
        // Only complete the first word (command's name)
        // If start != 0, we're completing arguments -> no builtin/external completion here
        if (start != 0) {
            return nullptr;
        }

        // Use rl_completion_matches which will call our generator
        return rl_completion_matches(text, builtin_generator);
    }

    void initialize() {
        // Use default completion delimiters but ensure tab triggers completion
        rl_attempted_completion_function = builtin_completion;
        // Ensure a space is appended after an unambiguous completion
        rl_completion_append_character = ' ';
    }
}
