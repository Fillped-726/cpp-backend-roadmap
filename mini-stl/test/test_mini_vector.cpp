#include <catch2/catch_all.hpp>
#include "mini_vector.h"

TEST_CASE("reserve dispatches to mmap for big type") {
    struct alignas(4096) Big { char buf[4096]; };  // 确保 >= 4096
    mini_vector<Big> vb;
    vb.reserve(1);
    // 只需要编译通过即可；后续可用捕获 stdout 断言
}

TEST_CASE("reserve dispatches to new[] for small type") {
    mini_vector<int> vi;
    vi.reserve(10);
}

TEST_CASE("mini_vector::at returns optional") {
    mini_vector<int> v;
    v.data_ = new int[3]{10, 20, 30};
    v.size_ = 3;

    REQUIRE(v.at(0).value() == 10);
    REQUIRE_FALSE(v.at(10).has_value());

    delete[] v.data_;
}
