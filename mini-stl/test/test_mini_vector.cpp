#include <catch2/catch_all.hpp>
#include "mini_vector.h"

TEST_CASE("mini_vector default constructed is empty") {
    mini_vector<int> v;
    CHECK(v.size() == 0);
    CHECK(v.capacity() == 0);
    CHECK(v.data() == nullptr);
}

// ------------------------------------------------
// 2. 构造 n 个默认元素
// ------------------------------------------------
TEST_CASE("mini_vector(n) creates n default objects") {
    mini_vector<int> v(5);
    CHECK(v.size() == 5);
    CHECK(v.capacity() >= 5);
    for (std::size_t i = 0; i < v.size(); ++i)
        CHECK(v.data()[i] == 0);          // int 默认构造为 0
}

// ------------------------------------------------
// 3. 仅移动类型可成功实例化，拷贝类型应编译失败
// ------------------------------------------------
// 该测试用例不写 REQUIRE，只要编译通过即可。
TEST_CASE("mini_vector accepts move-only type") {
    struct MoveOnly {
        MoveOnly() = default;
        MoveOnly(MoveOnly&&) noexcept {}
        MoveOnly& operator=(MoveOnly&&) noexcept { return *this; }
        MoveOnly(const MoveOnly&) = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;
    };
    mini_vector<MoveOnly> v(3);
    v.push_back(MoveOnly{});
    CHECK(v.size() == 4);
}

// ------------------------------------------------
// 4. push_back 触发扩容路径
// ------------------------------------------------
TEST_CASE("push_back triggers reallocation") {
    mini_vector<int> v;
    const int* old_ptr = v.data();
    for (int i = 0; i < 100; ++i) v.push_back(i);

    CHECK(v.size() == 100);
    CHECK(v.capacity() >= 100);
    // 地址应已改变
    CHECK(v.data() != old_ptr);
    // 内容保持
    for (int i = 0; i < 100; ++i)
        CHECK(v.data()[i] == i);
}

// ------------------------------------------------
// 5. 与 std::string 等复杂类型协作
// ------------------------------------------------
TEST_CASE("works with non-trivial type") {
    mini_vector<std::string> v;
    v.push_back("hello");
    v.push_back("world");
    CHECK(v.size() == 2);
    CHECK(v.at(0).value() == "hello");
    CHECK(v.at(1).value() == "world");
    CHECK_FALSE(v.at(2).has_value());
}

// ------------------------------------------------
// 6. reserve 不缩容
// ------------------------------------------------
TEST_CASE("reserve does not shrink") {
    mini_vector<int> v;
    v.reserve(100);
    REQUIRE(v.capacity() >= 100);

    v.reserve(50);
    CHECK(v.capacity() >= 100);   // 应保持原容量
}

// ------------------------------------------------
// 7. 异常安全：push_back/移动构造抛异常
// ------------------------------------------------
TEST_CASE("strong exception safety on move ctor throw") {
    struct ThrowOnMove {
        ThrowOnMove() = default;
        ThrowOnMove(ThrowOnMove&&) { throw 42; }
        ThrowOnMove& operator=(ThrowOnMove&&) { throw 42; return *this; }
    };
    mini_vector<ThrowOnMove> v(3);
    REQUIRE(v.size() == 3);

    // push_back 抛异常后 size/容量 应保持不变
    std::size_t old_size = v.size();
    std::size_t old_cap  = v.capacity();
    CHECK_THROWS_AS(v.push_back(ThrowOnMove{}), int);

    CHECK(v.size() == old_size);
    CHECK(v.capacity() == old_cap);
}

// ------------------------------------------------
// 8. 析构函数正确调用计数
// ------------------------------------------------
namespace {
        struct Counter {
            static int alive;
            Counter()  { ++alive; }
            ~Counter() { --alive; }
        };
        int Counter::alive = 0;
    }
TEST_CASE("destructor calls T::~T exactly size times") {
    {
        mini_vector<Counter> v(6);
        CHECK(Counter::alive == 6);
    }   // 离开作用域
    CHECK(Counter::alive == 0);
}

// ------------------------------------------------
// 9. 大对象分支编译通过（无运行期断言，仅编译）
// ------------------------------------------------
TEST_CASE("big type reserve compiles") {
    struct alignas(4096) Big { char buf[4096]; };
    mini_vector<Big> v;
    v.reserve(3);
}

// 测试用型别：基本 + 非默认构造 + 不可拷贝
struct NonCopy {
    int value;
    explicit NonCopy(int v = 0) : value(v) {}
    NonCopy(const NonCopy&)            = delete;
    NonCopy& operator=(const NonCopy&) = delete;
    NonCopy(NonCopy&&)                 = default;
    NonCopy& operator=(NonCopy&&)      = default;
};

TEST_CASE("mini_vector default ctor") {
    mini_vector<int> v;
    REQUIRE(v.size() == 0);
    REQUIRE(v.capacity() == 0);
}

TEST_CASE("mini_vector size ctor & iterators") {
    mini_vector<int> v(4);
    REQUIRE(v.size() == 4);
    REQUIRE(v.capacity() >= 4);
    REQUIRE(std::all_of(v.begin(), v.end(), [](int x) { return x == 0; }));
}

TEST_CASE("push_back / emplace_back") {
    mini_vector<NonCopy> v;
    v.emplace_back(1);
    v.emplace_back(2);
    REQUIRE(v.size() == 2);
    REQUIRE(v.at(0)->value == 1);
    REQUIRE(v.at(1)->value == 2);
}

TEST_CASE("pop_back returns optional") {
    mini_vector<std::string> v;
    v.push_back("a");
    v.push_back("b");
    REQUIRE(v.pop_back().value() == "b");
    REQUIRE(v.pop_back().value() == "a");
    REQUIRE_FALSE(v.pop_back().has_value());
}

TEST_CASE("at returns nullopt on out-of-range") {
    mini_vector<double> v(3);
    REQUIRE(v.at(3) == std::nullopt);
    REQUIRE(v.at(2).has_value());
}

TEST_CASE("reserve big type branch") {
    struct alignas(4096) Big { char buf[4096]; };
    mini_vector<Big> v;
    v.reserve(2);           // 触发打印，仅保证编译/运行
    REQUIRE(v.capacity() >= 2);
}

TEST_CASE("iterator loop") {
    mini_vector<int> v;
    for (int i = 0; i < 100; ++i) v.push_back(i);
    int sum = 0;
    for (int x : v) sum += x;
    REQUIRE(sum == 100 * 99 / 2);
}

/* ---------- 1. move-only 辅助类型 ---------- */
struct MoveOnlyNoexcept {
    int value;
    MoveOnlyNoexcept(int v = 0) : value(v) {}
    MoveOnlyNoexcept(MoveOnlyNoexcept&&) noexcept = default;
    MoveOnlyNoexcept& operator=(MoveOnlyNoexcept&&) noexcept = default;
    bool operator==(const MoveOnlyNoexcept& o) const { return value == o.value; }
};

/* ---------- 2. 测试用例 ---------- */

TEST_CASE("mini_vector works with move-only noexcept types") {
    mini_vector<MoveOnlyNoexcept> v;
    v.emplace_back(42);
    v.push_back(MoveOnlyNoexcept{100});

    REQUIRE(v.size() == 2);
    REQUIRE(v.at(0)->value == 42);
    REQUIRE(v.at(1)->value == 100);
}

TEST_CASE("reserve prints noexcept information") {
    // 肉眼观察 stdout：应出现 “T::T(T&&) noexcept = true”
    mini_vector<MoveOnlyNoexcept> v;
    REQUIRE_NOTHROW(v.reserve(64));
}

TEST_CASE("pop_back returns optional value") {
    mini_vector<MoveOnlyNoexcept> v;
    v.emplace_back(7);
    v.emplace_back(8);

    auto opt = v.pop_back();
    REQUIRE(opt.has_value());
    REQUIRE(opt->value == 8);
    REQUIRE(v.size() == 1);

    opt = v.pop_back();
    REQUIRE(opt.has_value());
    REQUIRE(opt->value == 7);
    REQUIRE(v.size() == 0);

    REQUIRE(v.pop_back() == std::nullopt);
}

TEST_CASE("const_iterator allows read-only range-for") {
    mini_vector<MoveOnlyNoexcept> v;
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);

    int sum = 0;
    for (const auto& x : v) {          // 使用 const_iterator
        sum += x.value;
    }
    REQUIRE(sum == 6);
}

static std::size_t g_sys_alloc_count = 0;
void* operator new(std::size_t sz) {
    g_sys_alloc_count++;
    return std::malloc(sz);
}
void operator delete(void* ptr) noexcept {
    std::free(ptr);
}
// 重置计数器的辅助函数
void reset_sys_alloc_count() {
    g_sys_alloc_count = 0;
}
std::size_t get_sys_alloc_count() {
    return g_sys_alloc_count;
}


TEST_CASE("mini_vector PMR - Monotonic Buffer Avoids System Alloc", "[pmr]") {
    reset_sys_alloc_count();
    const std::size_t buffer_size = 1 << 20; // 1MB
    char buf[buffer_size];
    std::pmr::monotonic_buffer_resource pool(buf, buffer_size);

    // 使用 PMR 构造 mini_vector
    mini_vector<int, std::pmr::polymorphic_allocator<int>> vec(&pool);

    const std::size_t num_elements = 10000;
    // 预计算所需内存大小 (粗略估计)
    const std::size_t estimated_bytes_needed = num_elements * sizeof(int);

    // 确保缓冲区足够大
    REQUIRE(buffer_size > estimated_bytes_needed);

    // 执行 reserve 操作
    vec.reserve(num_elements);

    // 检查是否没有发生系统级分配
    // 因为所有内存都应来自预分配的 buf
    REQUIRE(get_sys_alloc_count() == 0);

    // 可选：添加一些元素验证功能正常
    for (std::size_t i = 0; i < num_elements / 2; ++i) {
        vec.push_back(static_cast<int>(i));
    }
    REQUIRE(vec.size() == num_elements / 2);
    REQUIRE(vec.capacity() >= num_elements);
    // 再次确认 push_back 也没有触发系统分配 (取决于实现，但通常不会)
    REQUIRE(get_sys_alloc_count() == 0);
}

TEST_CASE("mini_vector PMR - Falls Back to System Alloc", "[pmr]") {
    reset_sys_alloc_count();

    // 不提供自定义内存资源，使用默认分配器 (通常链接到 new/delete)
    mini_vector<int, std::pmr::polymorphic_allocator<int>> vec;

    const std::size_t num_elements = 10000;
    // 执行 reserve 操作，预计将触发系统分配
    vec.reserve(num_elements);

    // 检查是否发生了系统级分配
    CHECK_NOTHROW(vec.reserve(num_elements)); // 至少有一次分配

    // 重置并测试 push_back 导致的扩容
    reset_sys_alloc_count();
    mini_vector<int, std::pmr::polymorphic_allocator<int>> vec2;
    // 先放入一些元素，使其接近默认容量（假设小于10000）
    for(int i = 0; i < 100; ++i) {
        vec2.push_back(i);
    }
    reset_sys_alloc_count(); // 重置计数，忽略初始分配
    // 触发扩容
    vec2.reserve(5000); // 这应该需要重新分配

}

TEST_CASE("mini_vector PMR - Move Semantics with PMR", "[pmr][move]") {
    reset_sys_alloc_count();
    const std::size_t buffer_size = 1 << 16; // 64KB
    char buf[buffer_size];
    std::pmr::monotonic_buffer_resource pool(buf, buffer_size);

    using PMRVector = mini_vector<int, std::pmr::polymorphic_allocator<int>>;
    PMRVector vec1(&pool);

    const std::size_t num_elements = 1000;
    for (int i = 0; i < static_cast<int>(num_elements); ++i) {
        vec1.push_back(i);
    }
    REQUIRE(vec1.size() == num_elements);
    REQUIRE(get_sys_alloc_count() == 0); // 确认内存来自池

    // 测试移动构造
    reset_sys_alloc_count(); // 移动不应分配新内存
    PMRVector vec2(std::move(vec1));

    // 原 vec1 应该为空
    REQUIRE(vec1.size() == 0);
    REQUIRE(vec1.data() == nullptr);
    // 新 vec2 应该拥有数据
    REQUIRE(vec2.size() == num_elements);
    REQUIRE(vec2.data() != nullptr);
    // 移动过程不应触发系统分配
    REQUIRE(get_sys_alloc_count() == 0);

    // 测试移动赋值
    reset_sys_alloc_count();
    PMRVector vec3; // 使用默认分配器
    vec3 = std::move(vec2);
    reset_sys_alloc_count(); // 重置，关注移动赋值

    // vec2 应该为空
    REQUIRE(vec2.size() == 0);
    REQUIRE(vec2.data() == nullptr);
    REQUIRE(vec3.size() == num_elements);
    if (!vec3.empty()) {
        REQUIRE(vec3.at(0).value() == 0);
        REQUIRE(vec3.at(num_elements - 1).value() == static_cast<int>(num_elements - 1));
    }
}