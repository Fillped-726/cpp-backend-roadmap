#pragma once
#include <iostream>

// 判断 T 是否是大对象
template <typename T>
inline constexpr bool is_big_type_v = (sizeof(T) >= 4096);

struct mini_vector {
    void reserve(std::size_t n) {
        if constexpr (is_big_type_v<int>) {   
            std::cout << "[stub] use mmap for big type\n";
        } else {
            std::cout << "[stub] use new[] for small type\n";
        }
        // TODO: real impl
    }
};