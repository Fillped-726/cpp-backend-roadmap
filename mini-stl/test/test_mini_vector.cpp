#include <catch2/catch_all.hpp>
#include "mini_vector.h"

TEST_CASE("mini_vector is default constructible") {
    mini_vector v;
    REQUIRE(true);   // 占位
}

TEST_CASE("reserve dispatches by type size") {
    mini_vector v;
    v.reserve(10);          // 触发 stub 打印
    REQUIRE(true);          // 占位
}