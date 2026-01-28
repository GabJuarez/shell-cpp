#include "builtins.hpp"
#include <readline/history.h>
#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>

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

    // Helper to read a history file and append non-empty lines to in-memory history
    static void read_history_file_into_memory(const std::string &path) {
        std::ifstream ifs(path);
        if (!ifs) return;
        std::string line;
        while (std::getline(ifs, line)) {
            // Skip empty/whitespace-only lines
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;
            add_history(line.c_str());
        }
    }

    void history(const std::vector<std::string> &args) {
        // Support `history -a <path>` (append), `history -w <path>` (write), `history -r <path>` (read)
        if (args.size() >= 2) {
            const std::string opt = args[0];
            const std::string path = args[1];
            if (opt == "-a") {
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
            if (opt == "-w") {
                // Write entire history to the specified file (truncate)
                // Use readline helper which writes the whole in-memory history
                if (!path.empty()) {
                    write_history(path.c_str());
                    // After writing the full history, mark last written index so -a won't reappend
                    last_history_length = history_length;
                }
                return;
            }
            if (opt == "-r") {
                if (!path.empty()) {
                    // Read file and append its non-empty lines to in-memory history
                    read_history_file_into_memory(path);
                    // Update last_history_length so -a won't reappend these lines
                    last_history_length = history_length;
                }
                return;
            }
        }

        // No options: print numbered history similar to bash
        for (int i = 0; i < history_length; ++i) {
            HIST_ENTRY *ent = history_get(i + 1);
            if (ent && ent->line) {
                // Format: right-aligned number in width 5, two spaces, then the line
                std::cout << std::setw(5) << (i + 1) << "  " << ent->line << std::endl;
            }
        }
    }
} // namespace sh::builtins
