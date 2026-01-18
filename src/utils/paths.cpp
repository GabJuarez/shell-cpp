#include <filesystem>
#include <optional>
#include <vector>
#include <sstream>
#include "../utils/paths.hpp"



namespace sh::paths {
    std::vector<std::string> get_all_entries(const std::filesystem::path &path) {
        std::vector<std::string> entries;
        try {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(path)) {
                entries.push_back(entry.path().string());
            }
        }
        catch (std::exception) {
            // Ignore permission errors and continue
        }
        return entries;
    }


    std::optional<std::string> get_first_entry(const std::string &command) {
        std::vector<std::filesystem::path> paths = sh::paths::get_paths();
        try {
            for (const auto &path : paths) {
            for (const auto &entry : std::filesystem::recursive_directory_iterator(path)) {
                if (is_regular_file(entry.path())) {
                    if (entry.path().stem().string().compare(command) == 0) {
                        return entry.path().string();
                    }
                }
            }
        }
        }
        catch (std::exception) {
            // Ignore permission errors and continue
        }
        return std::nullopt;
    }

    std::vector<std::filesystem::path> get_paths() {
        // PATHS
        std::vector<std::filesystem::path> paths;
        std::string r_paths = getenv("PATH");

        // string stream to split the path string using ':' as a del
        std::stringstream ss(r_paths);
        std::string temp_str;
        char del = ':';

        while (getline(ss, temp_str, del)) {
            paths.push_back(std::filesystem::path(temp_str));
        }
        return paths;
    }

}
