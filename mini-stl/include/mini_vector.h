#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include <optional>
#include <iostream>

/*---------------- 工具 ----------------*/
template <typename T>
inline constexpr bool is_big_type_v = (sizeof(T) >= 4096);

/* 1. move-only 断言：类型必须支持移动，且禁止拷贝 */
template <typename T>
concept move_aware = std::movable<T>;

/*---------------- mini_vector 定义 ----------------*/
template <typename T>
    requires move_aware<T>          // C++20 concept，简单明了
class mini_vector {
    T*          data_     = nullptr;
    std::size_t size_     = 0;
    std::size_t capacity_ = 0;

    /* 2. 运行时打印 T 的 move 构造是否 noexcept */
    static void log_move_noexcept() {
        constexpr bool noex = std::is_nothrow_move_constructible_v<T>;
        std::cout << "[mini_vector] T::T(T&&) noexcept = "
                  << std::boolalpha << noex << '\n';
    }

public:
    /*================ 构造 / 析构 ================*/
    mini_vector() = default;

    mini_vector(const mini_vector&)            = delete;
    mini_vector& operator=(const mini_vector&) = delete;

    explicit mini_vector(std::size_t n)
        : data_(static_cast<T*>(::operator new(sizeof(T) * n))),
          size_(n), capacity_(n) {
        std::uninitialized_value_construct_n(data_, n);
    }

    ~mini_vector() {
        std::destroy_n(data_, size_);
        ::operator delete(data_);
    }

    /*================ 迭代器 ================*/
    using iterator       = T*;
    using const_iterator = const T*;

    iterator       begin()       noexcept { return data_; }
    iterator       end()         noexcept { return data_ + size_; }
    const_iterator begin() const noexcept { return data_; }
    const_iterator end()   const noexcept { return data_ + size_; }
    const_iterator cbegin()const noexcept { return begin(); }
    const_iterator cend()  const noexcept { return end(); }

    /*================ reserve ================*/
    void reserve(std::size_t new_cap) {
        if (new_cap <= capacity_) return;

        log_move_noexcept();          // 关键日志：先打印 noexcept 信息

        T* new_data = static_cast<T*>(::operator new(sizeof(T) * new_cap));
        std::size_t i = 0;
        try {
            for (; i < size_; ++i)
                ::new (static_cast<void*>(new_data + i))
                    T(std::move_if_noexcept(data_[i]));
        } catch (...) {
            std::destroy_n(new_data, i);
            ::operator delete(new_data);
            throw;
        }
        std::destroy_n(data_, size_);
        ::operator delete(data_);
        data_     = new_data;
        capacity_ = new_cap;
    }

    /*================ push_back / emplace_back ================*/
    template <typename U>
    void push_back(U&& value) {
        if (size_ == capacity_)
            reserve(std::max<std::size_t>(capacity_ * 2, 1));
        ::new (static_cast<void*>(data_ + size_))
            T(std::forward<U>(value));
        ++size_;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        if (size_ == capacity_)
            reserve(std::max<std::size_t>(capacity_ * 2, 1));
        ::new (static_cast<void*>(data_ + size_))
            T(std::forward<Args>(args)...);
        ++size_;
    }

    /*================ pop_back ================*/
    std::optional<T> pop_back() {
        if (size_ == 0) return std::nullopt;
        T val = std::move(data_[--size_]);
        data_[size_].~T();
        return val;
    }

    /*================ 其它辅助 ================*/
    std::optional<T> at(std::size_t i) {
        return (i < size_) ? std::optional<T>(std::move(data_[i])) : std::nullopt;
    }
    std::size_t size()     const noexcept { return size_; }
    std::size_t capacity() const noexcept { return capacity_; }
    T*       data()        noexcept { return data_; }
    const T* data()  const noexcept { return data_; }
};