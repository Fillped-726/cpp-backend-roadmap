#pragma once
#include <string_view>
#include <vector>
#include <optional>

inline std::optional<std::vector<std::string_view>>
split_view(std::string_view s, char delim) {
    if (s.empty()) return std::nullopt;

    std::vector<std::string_view> out;
    std::size_t first = 0;
    while (first < s.size()) {
        const auto second = s.find(delim, first);
        if (second == std::string_view::npos) {
            out.emplace_back(s.substr(first));
            break;
        }
        out.emplace_back(s.substr(first, second - first));
        first = second + 1;
    }
    return out;
}