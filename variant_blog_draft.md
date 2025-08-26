# C++17 std::variant 实战：消灭 union 与 switch

> 今天，我们从一个简单需求出发：函数需要返回「int、double 或 std::string」之一，却不想再写 `union + switch + enum`。C++17 的 `std::variant` 让这件事变得优雅且零开销。

## 1. 问题背景

传统做法：

```cpp
enum class Type { INT, DOUBLE, STR };
struct Value {
    Type tag;
    union {
        int    i;
        double d;
        char*  s;      // 还要管内存
    } u;
};
```

- 手动 `switch(tag)` 解析  
- 忘记写 `break` 就 UB  
- 资源管理噩梦（char* 需要拷贝/释放）

## 2. variant 基础

一行定义，值语义：

```cpp
using Value = std::variant<int, double, std::string>;
Value v{3.14};          // 存 double
assert(v.index() == 1); // 0:int, 1:double, 2:string
```

大小：`max(sizeof(Ts)...) + 1` 字节判别式，**无额外虚函数开销**。

## 3. visit 的三种写法

### 3.1 通用 lambda（推荐）

```cpp
std::visit([](auto&& x){ std::cout << x << '\n'; }, v);
```

编译器自动生成三份函数体，**零运行时分支**。

### 3.2 结构体重载

```cpp
struct Printer {
    void operator()(int    i) const { std::cout << "int:"    << i; }
    void operator()(double d) const { std::cout << "double:" << d; }
    void operator()(const std::string& s) const { std::cout << "str:" << s; }
};
std::visit(Printer{}, v);
```

### 3.3 泛型+if constexpr（复杂逻辑）

```cpp
std::visit([](auto&& x){
    using T = std::decay_t<decltype(x)>;
    if constexpr (std::is_same_v<T, int>)   compute_int(x);
    else if constexpr (std::is_same_v<T, std::string>) compute_str(x);
}, v);
```

## 4. 常见错误

| 场景 | 结果 | 解决 |
|---|---|---|
| `std::get<int>(v)` 当 v 存 string | 抛 `std::bad_variant_access` | 用 `std::get_if` 或 `visit` |
| 赋值抛异常 | 进入 *valueless* 状态 | 保证所有备选类型异常安全 |
| 单参数构造歧义 | 可能选错类型 | `std::in_place_type<T>` 显式构造 |

## 5. 与 optional 组合

解析配置项：

```cpp
std::optional<Value> parse(std::string_view s) {
    if (s.empty()) return std::nullopt;
    if (s.find('.') != s.npos) return std::stod(std::string(s));
    if (s[0] == '"') return std::string(s.substr(1, s.size()-2));
    return std::stoi(std::string(s));
}
```

链式调用：

```cpp
auto v = parse(input)
            .transform([](Value val){ return std::visit(scale, val); });
```

## 6. 小结

- `variant` = 类型安全 `union`，零开销，值语义  
- `visit` 消灭 `switch`，编译期多态  
- 与 `optional` 配合，形成「空值 + 多类型」的组合拳

今天，我们在 `mini_vector` 里只加了 10 行代码，就让接口同时支持三种标量类型，而调用端再也看不到 `switch` 的影子。

> 让编译器写 `switch`，我们写业务逻辑。