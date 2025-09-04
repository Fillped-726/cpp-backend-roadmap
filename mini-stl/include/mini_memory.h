#pragma once
#include <memory>
#include <type_traits>
#include <cstddef>
#include <new>

namespace my {

// 自定义删除器，用于 placement new 构造的数组
template <typename T>
struct placement_delete {
    std::size_t count;
    explicit placement_delete(std::size_t n) : count(n) {}

    void operator()(T* ptr) {
        if (ptr) {
            // 手动调用析构函数
            for (std::size_t i = 0; i < count; ++i) {
                ptr[i].~T();
            }
            // 释放原始内存
            ::operator delete[](ptr);
        }
    }
};

/* -------------- 单对象版本 -------------- */
template <class T>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
make_unique_for_overwrite() {
    void* raw = ::operator new(sizeof(T));
    try {
        // Placement new 构造对象
        T* obj = new (raw) T;
        // 使用默认删除器 std::default_delete<T>
        return std::unique_ptr<T>(obj);
    } catch (...) {
        // 如果构造函数抛出异常，则释放内存
        ::operator delete(raw);
        throw;
    }
}

/* -------------- 未知边界数组版本 (修复版) -------------- */
template <class T>
typename std::enable_if<std::is_array<T>::value &&
                        std::extent<T>::value == 0,   // T is U[]
                        std::unique_ptr<T, placement_delete<typename std::remove_extent<T>::type>>>::type
make_unique_for_overwrite(std::size_t n) {
    using U = typename std::remove_extent<T>::type;
    if (n == 0) {
        return std::unique_ptr<T, placement_delete<U>>(nullptr, placement_delete<U>(0));
    }

    void* raw = ::operator new[](sizeof(U) * n);
    U* obj_ptr = static_cast<U*>(raw); // 转换为对象指针类型
    std::size_t constructed = 0;
    try {
        // Placement new 构造数组中的每个对象
        for (std::size_t i = 0; i < n; ++i) {
            new (obj_ptr + i) U; // Placement new for U
            ++constructed;
        }
        // 使用自定义删除器
        return std::unique_ptr<T, placement_delete<U>>(obj_ptr, placement_delete<U>(n));
    } catch (...) {
        // 如果任何构造函数抛出异常，则析构已构造的对象并释放内存
        for (std::size_t i = 0; i < constructed; ++i) {
            obj_ptr[i].~U();
        }
        ::operator delete[](raw);
        throw;
    }
}


/* -------------- 已知边界数组（禁止）-------------- */
template <class T>
typename std::enable_if<std::is_array<T>::value &&
                        std::extent<T>::value != 0>::type
make_unique_for_overwrite() = delete;   // 例如 int[3] 不提供

} // namespace my