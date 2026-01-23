#include "helpers.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace helpers {
    // Function to trim leading and trailing whitespace from a string
    // it checks for the char before the end iterator cause end is not taken into
    // account anyway
    std::string trim(const std::string &str) {
        auto start = str.begin();
        auto end = str.end();

        while (start != end && std::isspace(*start)) {
            ++start;
        }

        while (end != start && std::isspace(*(end - 1))) {
            --end;
        }

        return {start, end};
    }

    std::vector<std::string> parser(const std::string &r_args) {
        // Final vector and variables needed
        std::vector<std::string> res;
        std::string buffer;
        bool inside_single_quotes = false;
        bool inside_double_quotes = false;
        bool escape = false;


        for (int i = 0; i < r_args.length(); i++) {
            if (!buffer.empty() && r_args[i] == '\n') {
                res = {};
                return res;
            } else if (!inside_double_quotes && !inside_single_quotes && r_args[i] == '>') {
                if (!buffer.empty()) {
                    if (buffer.length() == 1 && buffer[0] == '1') {
                        buffer = "";
                        buffer += r_args[i];
                        res.push_back(buffer);
                        buffer = "";
                        continue;
                    } else if (buffer.length() == 1 && buffer[0] == '2') {
                        buffer += r_args[i];
                        res.push_back(buffer);
                        buffer = "";
                        continue;
                    }
                    res.push_back(buffer);
                    buffer = "";
                    buffer += r_args[i];
                    res.push_back(buffer);
                    buffer = "";
                    continue;
                }
            } else if ((escape && inside_double_quotes && (
                            r_args[i] == '\"' || r_args[i] == '$' || r_args[i] == '`' || r_args[i] == '\\'))
                       || (!inside_double_quotes && !inside_single_quotes && escape)) {
                if (r_args[i] == 'n' && inside_double_quotes) {
                    res.push_back(buffer);
                    buffer = "<newline>";
                    res.push_back(buffer);
                    buffer = "";
                    escape = false;
                    continue;
                }
                buffer += r_args[i];
                escape = false;
                continue;
            } else if (inside_double_quotes && escape) {
                buffer += '\\';
                buffer += r_args[i];
                escape = false;
                continue;
            } else if ((r_args[i] == '\\' && !inside_double_quotes && !inside_single_quotes) ||
                       (inside_double_quotes && r_args[i] == '\\')) {
                escape = true;
                continue;
            } else if (r_args[i] == '\'' && !inside_double_quotes) {
                inside_single_quotes = !inside_single_quotes;
                continue;
            } else if (r_args[i] == '\"' && !inside_single_quotes) {
                inside_double_quotes = !inside_double_quotes;
                continue;
            } else if (r_args[i] == ' ' || r_args[i] == '\t') {
                if (!buffer.empty() && !inside_single_quotes && !inside_double_quotes) {
                    res.push_back(buffer);
                    buffer = "";
                } else if (inside_single_quotes || inside_double_quotes) {
                    buffer += r_args[i];
                }
                continue;
            } else if (inside_single_quotes || inside_double_quotes) {
                buffer += r_args[i];
                continue;
            }
            buffer += r_args[i];
        }
        if (!buffer.empty()) {
            res.push_back(buffer);
        }
        return res;
    }
} // namespace helpers
