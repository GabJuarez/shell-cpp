#include "helpers.hpp"
#include "../parser/parser.hpp"
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
        // Forward to the new parser module (implementation preserved there)
        return sh::parser::parse(r_args);
    }
} // namespace helpers
