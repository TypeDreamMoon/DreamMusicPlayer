// DreamLyricParser - Utility functions
// SPDX-License-Identifier: MIT

#pragma once

#include <cctype>
#include <string>
#include <string_view>

namespace dream_lyric_parser {
namespace utils {

[[nodiscard]] inline std::string Trim(std::string_view text) {
    if (text.empty()) {
        return {};
    }
    auto begin = text.begin();
    auto end = text.end();
    while (begin != end && std::isspace(static_cast<unsigned char>(*begin))) {
        ++begin;
    }
    while (begin != end && std::isspace(static_cast<unsigned char>(*(end - 1)))) {
        --end;
    }
    if (begin >= end) {
        return {};
    }
    return std::string(begin, end);
}

[[nodiscard]] inline std::string Trim(const std::string& value) {
    return Trim(std::string_view(value));
}

[[nodiscard]] inline bool StartsWith(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

[[nodiscard]] inline bool EndsWith(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

}  // namespace utils
}  // namespace dream_lyric_parser

