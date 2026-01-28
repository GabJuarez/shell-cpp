#include "builtins.hpp"
#include <readline/history.h>
#include <fstream>
#include <iostream>
#include <string>

namespace sh::builtins {
    // We track the position (number of entries) of the last written/read history
    // so that repeated `history -a` only write new entries since the last write.
    static int last_history_length = 0;

    void history_init() {
        // Initialize last_history_length to the current history length
        last_history_length = history_length;
    }

    // Helper to append entries from history to file starting at index `start_idx`.
    static void append_history_range_to_file(const std::string &path, int start_idx) {
        std::ofstream ofs(path, std::ios::app);
        if (!ofs) {
            // silently ignore failures to open file as existing code does for read/write
            return;
        }
        // readline history list is 0-based, history_length is total entries
        for (int i = start_idx; i < history_length; ++i) {
            HIST_ENTRY *ent = history_get(i + 1); // history_get is 1-based
            if (ent && ent->line) {
                ofs << ent->line << std::endl;
            }
        }
    }

    void history(const std::vector<std::string> &args) {
        // Support at least `history -a <path>`
        if (args.size() >= 2 && args[0] == "-a") {
            const std::string path = args[1];
            // Append only entries that weren't previously appended
            int start_idx = last_history_length;
            if (start_idx < 0) start_idx = 0;
            if (start_idx < history_length) {
                append_history_range_to_file(path, start_idx);
                // After appending, update last_history_length to current
                last_history_length = history_length;
            }
            return;
        }

        // For any other usage, fallback to printing the history to stdout
        // (simple behavior to avoid breaking tests that don't use other forms)
        for (int i = 0; i < history_length; ++i) {
            HIST_ENTRY *ent = history_get(i + 1);
            if (ent && ent->line) std::cout << ent->line << std::endl;
        }
    }
} // namespace sh::builtins
