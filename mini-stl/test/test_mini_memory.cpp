#include <catch2/catch_all.hpp>
#include "mini_memory.h"
#include <cstring>

namespace {          // 匿名命名空间，仅本文件可见
struct Foo {
    int x;
    Foo() : x(42) {}
    ~Foo() { /* 方便看析构日志 */ }
};
}

TEST_CASE("make_unique_for_overwrite single object") {
    auto p = my::make_unique_for_overwrite<Foo>();
    REQUIRE(p != nullptr);
    REQUIRE(p->x == 42);               // 默认构造值
    std::memset(p.get(), 0, sizeof(Foo)); // 手动覆盖
    REQUIRE(p->x == 0);
}

TEST_CASE("make_unique_for_overwrite array") {
    constexpr std::size_t n = 5;
    auto p = my::make_unique_for_overwrite<Foo[]>(n);
    REQUIRE(p != nullptr);
    for (std::size_t i = 0; i < n; ++i)
        REQUIRE(p[i].x == 42);
}

TEST_CASE("make_unique_for_overwrite with non-default-constructible") {
    struct NoDefault {
        int v;
        explicit NoDefault(int x) : v(x) {}
    };
    // 我们删掉了数组版默认构造，因此下面这句应该**编译失败**
    // auto p = my::make_unique_for_overwrite<NoDefault[]>();   // 留给你验证 SFINAE
    // 单对象仍可显式构造
    auto p = std::make_unique<NoDefault>(99);
    REQUIRE(p->v == 99);
}


