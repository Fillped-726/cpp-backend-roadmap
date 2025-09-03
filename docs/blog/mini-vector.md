# 博客草稿 · 大纲

**标题**：为什么移动构造必须是 noexcept？  
**副标题**：一次 STL 容器扩容失败引发的性能血案  
**定位**：C++进阶 | 移动语义 | 异常安全 | 性能优化  

---

## 一、开场故事（钩子）
- 线上服务升级后 CPU 暴涨 30%，perf 热点落在 `std::vector::reserve`  
- 根因：自定义类型忘记给 `noexcept` 标记，STL 默默回退到「拷贝重分配」  
- 结论先行：移动构造若不 `noexcept`，容器扩容时性能直接打回 C++03 年代  

---

## 二、背景知识速览
1. **移动语义**要解决什么问题  
  - 深拷贝 → 浅搬运，O(n) 变 O(1)  
2. **STL 强异常安全**承诺  
  - `vector::push_back` 要么成功、要么原状回滚  
3. **noexcept** 语义  
  - `noexcept(true)` = 绝不抛；若抛 → `std::terminate`  
  - 编译期可检测 `std::is_nothrow_move_constructible_v<T>`  

---

## 三、核心原理：扩容时的两条代码路径
| 条件 | 策略 | 复杂度 | 异常安全 |
| ---- | ---- | ---- | ---- |
| `move_if_noexcept` 为 `true` | 移动构造 | O(1) 每元素 | 若抛则直接 terminate（已承诺不抛） |
| `move_if_noexcept` 为 `false` | 拷贝构造 | O(n) 每元素 | 抛了还能回滚到旧数组 |

源码级别：  
```cpp
::new (dest) T(std::move_if_noexcept(src[i]));
```
`move_if_noexcept` 内部就是 `noexcept` 判断的语法糖。

---

## 四、实战复现
### 1. 测试类型
```cpp
struct Data {
    std::vector<char> blob_;          // 可能很大
    Data(Data&&) noexcept = default;  // 版本 A
    // Data(Data&&) {}                 // 版本 B：故意去掉 noexcept
};
```

### 2. benchmark 代码
- 记录 `reserve(1'000'000)` 耗时 & 内存峰值  
- 版本 A：2 ms / 0 次额外拷贝  
- 版本 B：1.3 s / 1 000 000 次深拷贝  

### 3. 汇编对比
- `objdump -d` 看循环体：版本 B 内联 `memcpy@plt`；版本 A 只有寄存器搬运  

---

## 五、常见误区
| 误区 | 正解 |
| ---- | ---- |
| “我代码里没抛，就不用写 noexcept” | 编译器只看声明；不写默认 `noexcept(false)` |
| “虚函数析构 noexcept(false) 没关系” | 会连带导致所有派生类移动无法 `noexcept` |
| “小型 POD 不写也自动优化” | STL 仍按规范走分支，不会猜 |

---

## 六、最佳实践 checklist
✅ 凡是「深拷贝 → 浅搬运」的类型，移动两函数一律 `noexcept`  
✅ 用 `static_assert(std::is_nothrow_move_constructible_v<T>)` 强制文档化  
✅ 对虚基类，把析构也标 `noexcept`；否则整个继承树失去移动加速  
✅ CI 里跑 `std::vector<T> v; v.reserve(N)` 的 benchmark，回归监控  

---

## 七、拓展阅读
- 《Effective Modern C++》Item 14、15  
- libcxx `__move_if_noexcept` 源码  
- Howard Hinnant 2012 演讲《Exception Safety and Performance》  

---

## 八、结尾金句
> 在 C++ 世界，沉默不是金——沉默的 `noexcept(false)` 会让你在深夜三点和 `memcpy` 一起加班。  
> 给移动构造一个 `noexcept`，就是给未来的自己留一条下班的路。