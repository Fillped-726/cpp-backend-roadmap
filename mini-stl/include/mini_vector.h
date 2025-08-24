#pragma once
#include <iostream>

template <typename T>
inline constexpr bool is_big_type_v = (sizeof(T) >= 4096);

template <typename T>
struct mini_vector {
    void reserve(std::size_t n) {
        if constexpr (is_big_type_v<T>) {
            std::cout << "[mmap] reserve " << n << " big objects\n";
            // TODO: mmap stub
        } else {
            std::cout << "[new[]] reserve " << n << " small objects\n";
            // TODO: new[] stub
        }
    }
};