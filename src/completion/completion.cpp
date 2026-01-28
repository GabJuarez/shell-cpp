#include <readline/readline.h>
#include <readline/history.h>
#include <cstring>
#include <cstdlib>
#include <string>

#include "../completion/completion.hpp"
#include "../commands/builtins.hpp"

namespace completion {
    // Generator called by readline to produce possible completions one by one
    static char *builtin_generator(const char *text, int state) {
        static size_t list_index;
        static size_t len;

        if (state == 0) {
            list_index = 0;
            len = std::strlen(text);
        }

        // Iterate through builtins array and return matches incrementally
        for (; list_index < (sizeof(sh::builtins::builtins) / sizeof(std::string)); ++list_index) {
            const std::string &candidate = sh::builtins::builtins[list_index];
            if (candidate.compare(0, len, text) == 0) {
                // readline expects malloc'd string
                char *match = static_cast<char *>(std::malloc(candidate.size() + 1));
                if (!match) return nullptr;
                std::strcpy(match, candidate.c_str());
                ++list_index; // advance for next call
                return match;
            }
        }

        // No more matches
        return nullptr;
    }

    // This function is called by readline to produce the matches for the current word
    static char **builtin_completion(const char *text, int start, int end) {
        // Only complete the first word (command's name)
        // If start != 0, we're completing arguments -> no builtin completion here
        if (start != 0) {
            return nullptr;
        }

        // Use rl_completion_matches which will call our generator
        return rl_completion_matches(text, builtin_generator);
    }

    void initialize() {
        // Use default completion delimiters but ensure tab triggers completion
        rl_attempted_completion_function = builtin_completion;
    }
}
