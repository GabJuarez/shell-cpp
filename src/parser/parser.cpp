#include "parser.hpp"

#include <string>
#include <vector>

namespace sh::parser {
    std::vector<std::string> parse(const std::string &r_args) {
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
                if (buffer.empty()) {
                    buffer += r_args[i];
                    continue;
                }

                if (buffer.length() == 1 && (r_args[i - 1] == '1' || r_args[i - 1] == '2')) {
                    buffer += r_args[i];
                    continue;
                } else if (buffer.length() == 2 && (
                               (r_args[i - 2] == '1' || r_args[i - 2] == '2') && r_args[i - 1] == '>')) {
                    buffer += r_args[i];
                    res.push_back(buffer);
                    buffer.clear();
                    continue;
                }
                buffer += r_args[i];
                res.push_back(buffer);
                buffer.clear();
                continue;
            } else if (!inside_double_quotes && !inside_single_quotes && r_args[i] == '|') {
                // Treat pipe as its own token; flush any current buffer first
                if (!buffer.empty()) {
                    res.push_back(buffer);
                    buffer.clear();
                }
                std::string pipeTok(1, '|');
                res.push_back(pipeTok);
                continue;
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
            } else if (r_args[i] == '"' && !inside_single_quotes) {
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
} // namespace sh::parser

