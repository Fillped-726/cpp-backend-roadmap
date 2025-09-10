#pragma once
#include <memory>
#include <utility>
#include <type_traits>
#include <optional>
#include <iostream>
// 引入 PMR 头文件
#include <memory_resource>

/*---------------- 工具 ----------------*/
template <typename T>
inline constexpr bool is_big_type_v = (sizeof(T) >= 4096);

/* 1. move-only 断言：类型必须支持移动 */
template <typename T>
concept move_aware = std::movable<T>;

/*---------------- mini_vector 定义 (支持 PMR) ----------------*/
// 默认使用 std::pmr::polymorphic_allocator
template <typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
    requires move_aware<T>
class mini_vector {
private:
    // 使用模板参数指定的 Allocator 类型
    using allocator_type = Allocator;
    // 确保 allocator_type 是 polymorphic_allocator 的实例，或者我们能从中获取内存资源
    // 这里我们假设 Allocator 是 std::pmr::polymorphic_allocator<T> 或兼容接口
    static_assert(std::is_same_v<Allocator, std::pmr::polymorphic_allocator<T>>,
                  "Allocator must be std::pmr::polymorphic_allocator<T> for this implementation");

    T*              data_     = nullptr;
    std::size_t     size_     = 0;
    std::size_t     capacity_ = 0;
    allocator_type  alloc_; // 存储分配器实例

    /* 2. 运行时打印 T 的 move 构造是否 noexcept */
    static void log_move_noexcept() {
        constexpr bool noex = std::is_nothrow_move_constructible_v<T>;
        std::cout << "[mini_vector] T::T(T&&) noexcept = "
                  << std::boolalpha << noex << '\n';
    }

    // 辅助函数：获取原始内存指针（用于与原版兼容的 destroy 等操作）
    T* to_address(T* ptr) const noexcept {
        // 对于普通指针，std::to_address 直接返回它
        // 如果 alloc_ 返回 fancy pointer, std::to_address 会处理解引用
        // 但我们这里假设 T* 就是普通指针
        return ptr;
    }

public:

    bool empty() const noexcept {
    return size_ == 0;
    }
    /*================ 构造 / 析构 ================*/
    // 默认构造函数：使用默认构造的分配器 (通常指向 new_delete_resource)
    mini_vector()
    : alloc_(std::pmr::new_delete_resource()) {}
    // 接受内存资源指针的构造函数
    // 允许用户指定用于分配内存的资源
    explicit mini_vector(std::pmr::memory_resource* resource)
        : alloc_(resource) {}

    // 接受分配器实例的构造函数
    explicit mini_vector(const allocator_type& alloc)
        : alloc_(alloc) {}

    // 接受大小和可选分配器的构造函数
    explicit mini_vector(std::size_t n, const allocator_type& alloc = allocator_type{})
        : alloc_(alloc) {
        // 使用分配器分配内存
        data_ = alloc_.allocate(n);
        capacity_ = n;
        size_ = n;
        // 在分配的内存中构造对象
        std::uninitialized_value_construct_n(data_, n);
    }

    // 接受大小和内存资源的构造函数
    explicit mini_vector(std::size_t n, std::pmr::memory_resource* resource)
        : mini_vector(n, allocator_type(resource)) {} // 委托给上面的构造函数


    mini_vector(const mini_vector&)            = delete;
    mini_vector& operator=(const mini_vector&) = delete;

    // Move constructor
    mini_vector(mini_vector&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)),
          size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)),
          alloc_(std::move(other.alloc_)) {
        // 如果 alloc_ 有状态且与 other.alloc_ 不同，行为可能未定义
        // 对于 std::pmr::polymorphic_allocator，只要内存资源相同，就没问题
    }

    // Move assignment
    mini_vector& operator=(mini_vector&& other) noexcept {
        if (this != &other) {
            std::destroy_n(data_, size_);
            if (data_) alloc_.deallocate(data_, capacity_);

            data_     = std::exchange(other.data_, nullptr);
            size_     = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);

            // 重新构造 allocator，避开已删除的赋值
            std::construct_at(&alloc_, other.alloc_.resource());
        }
        return *this;
    }


    ~mini_vector() {
        // 使用分配器的 destroy 和 deallocate
        std::destroy_n(data_, size_);
        if (data_) {
            alloc_.deallocate(data_, capacity_);
        }
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

        log_move_noexcept();

        // 使用分配器分配新内存
        T* new_data = alloc_.allocate(new_cap);
        std::size_t i = 0;
        try {
            for (; i < size_; ++i)
                ::new (static_cast<void*>(new_data + i))
                    T(std::move_if_noexcept(data_[i]));
        } catch (...) {
            // 使用分配器销毁和释放新分配的内存
            std::destroy_n(new_data, i);
            alloc_.deallocate(new_data, new_cap);
            throw;
        }
        // 销毁并释放旧内存
        std::destroy_n(data_, size_);
        if (data_) {
            alloc_.deallocate(data_, capacity_);
        }
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

    // 获取分配器实例
    allocator_type get_allocator() const noexcept {
        return alloc_;
    }
};



