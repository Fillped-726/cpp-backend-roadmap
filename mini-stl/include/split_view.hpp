#pragma once
#include <string_view>
#include <vector>

// O(n) 单次扫描，不拷贝字符
inline std::vector<std::string_view>
split_view(std::string_view s, char delim) {
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