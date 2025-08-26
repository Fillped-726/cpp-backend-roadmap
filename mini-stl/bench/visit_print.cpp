#include "../include/value.hpp"
#include <iostream>

// 通用 lambda：编译期生成分派代码，零运行时分支
auto print_visitor = [](auto&& v) {
    std::cout << v << '\n';
};

int main() {
    Value v1{42};          // int
    Value v2{3.14};        // double
    Value v3{"hello"};     // std::string

    std::visit(print_visitor, v1);
    std::visit(print_visitor, v2);
    std::visit(print_visitor, v3);
}