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
