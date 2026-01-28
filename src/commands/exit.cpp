#include <cstdlib>
#include <readline/history.h>
#include <string>

namespace sh::builtins {
    void exit1() {
        // Determine history file path the same way main does
        const char *hist_env_std = std::getenv("HISTFILE");
        const char *hist_env_alt = std::getenv("SHELL_HISTORY_FILE");
        std::string history_file;
        if (hist_env_std && hist_env_std[0] != '\0') {
            history_file = hist_env_std;
        } else if (hist_env_alt && hist_env_alt[0] != '\0') {
            history_file = hist_env_alt;
        } else {
            const char *home = std::getenv("HOME");
            history_file = (home ? std::string(home) : std::string(".")) + "/.shell_history";
        }

        // Attempt to write history (silently ignore failure)
        write_history(history_file.c_str());

        // Exit the process
        exit(0);
    }
}
