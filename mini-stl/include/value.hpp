#pragma once
#include <variant>
#include <string>

// alias template：以后换类型只改一行
using Value = std::variant<int, double, std::string>;