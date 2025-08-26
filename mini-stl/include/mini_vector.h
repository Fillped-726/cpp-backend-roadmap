#pragma once
#include <iostream>
#include <optional>

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

    // 新增：越界时返回 nullopt，合法时返回值
    std::optional<T> at(std::size_t i) const {
        if (i >= size_) return std::nullopt;
        return data_[i];
    }

    // 下面 stub，保持编译通过
    T* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};