# 一周 C++17 速成笔记 & 实战代码库  
> Day 1 ~ Day 7 学习轨迹、踩坑记录与可运行代码

---
## 📅 时间轴总览
| Day | 主题 | 关键实现 | 备注 |
|---|---|---|---|
|1|auto 推导 & 初始化列表|`is_big_type_v<T>` + `reserve()`|模板实参推导陷阱|
|2|结构化绑定|`split_view` + `optional<vector<string_view>>`|range-for + tuple|
|3|string_view & chrono|`elapsed()` + `split_key()`|零拷贝、字面量|
|4|optional|`mini_vector<T>::at()`|空值语义|
|5|alias template & variant|`using Value = variant<int,double,string>`|消灭 union|
|6|mini_vector 核心|RAII + 异常安全|手写 pointer 迭代器|
|7|增强功能|iterator / pop_back / emplace_back|80 % 覆盖率|

---
## 🧩 核心代码速查

### ① auto 推导
```cpp
auto x{1};          // C++17 int
auto y = {1};       // std::initializer_list<int>
```

### ② 结构化绑定
```cpp
for (auto [key,val] : my_map) { ... }
```

### ③ string_view 与 chrono
```cpp
using namespace std::chrono_literals;
auto d = 1500ms + 2s;
```

### ④ optional
```cpp
std::optional<T> at(std::size_t i) const {
    return (i < size_) ? data_[i] : std::nullopt;
}
```

### ⑤ alias template
```cpp
template<class T>
using remove_const_t = typename std::remove_const<T>::type;
```

### ⑥ variant vs union
```cpp
using Value = std::variant<int,double,std::string>;
std::visit([](auto&& x){ std::cout << x; }, v);
```

### ⑦ mini_vector（完整）
```cpp
template <typename T>
class mini_vector {
    T*          data_     = nullptr;
    std::size_t size_     = 0;
    std::size_t capacity_ = 0;
public:
    // 默认、size、析构、reserve、push_back、emplace_back、pop_back
    // iterator、optional<T> at(...)
};
```

---
## 🛠 本周实战仓库

```
mini-stl/
├─ include/
│  ├─ mini_vector.h        // Day 1~7 全部功能
│  ├─ split_view.hpp       // Day 2
│  ├─ value.hpp            // Day 5
│  └─ chrono_utils.hpp     // Day 3
├─ test/
│  └─ test_mini_vector.cpp // 80 %+ 覆盖率
├─ bench/
│  ├─ bench_split.cpp
│  └─ bench_variant_vs_virtual.cpp
└─ docs/
   └─ weekly_cpp17_notes.md
```

---
## 🚀 一键运行

```bash
git clone https://github.com/<you>/mini-stl.git
cd mini-stl
cmake -B build && cmake --build build
ctest -C Debug               # 单元测试
valgrind --leak-check=full ./build/test_mini_vector
./build/bench_variant_vs_virtual
```

---
## 📈 性能快照 (Clang 17, -O3)

| 场景 | 耗时 | 说明 |
|---|---|---|
| `virtual` 动态分派 | 9.8 ns | 虚函数表 |
| `variant` + `visit` | 2.1 ns | 编译期多态 |

---
## ⚠️ 踩坑速记

- `auto x{1};` 在 C++14/17 行为不同  
- `string_view` 绝不能指向临时  
- `variant` 赋值抛异常 → valueless_by_exception  
- `mini_vector` 必须 `destroy_n` + `operator delete`

---
## 🏁 本周收获

1. **零成本抽象**：`variant`+`visit` 取代 `switch`  
2. **RAII 手写**：new/delete + try/catch 保证异常安全  
3. **测试驱动**：Catch2 + Valgrind 一条龙  
4. **文档即笔记**：markdown + GitHub Pages 即时复习

> 让编译器写分支，我们写逻辑。
```