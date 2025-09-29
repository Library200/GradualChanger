# GradualChanger

一个 **单头文件、零依赖、类型安全** 的 C++17 渐变库。  
让任何变量（`int`、`float`、`double`、`bool`、自定义 vec3…）在 **指定帧数 / 时钟** 内平滑地过渡到目标值，而你只需要每帧调用一次 `GradualChangerBase::update()`。

---

## ✨ 特性

- **头文件-only** —— `#include "GradualChanger.h"` 即可  
- **类型泛化** —— 内置支持所有算术类型；自定义类型只需特化 `GradualTraits<T>`  
- **无动态分配**（用户侧）—— 内部用侵入链表管理生命周期，完成自动 `delete`  
- **帧率无关** —— 依赖外部时钟（`const int&`），可匀速、可变速、可暂停  
- **线程安全** —— 单线程模型，无锁；多线程请自行加锁或每线程独立时钟  
- **现代 C++** —— 基于 `std::variant` + CRTP，编译期类型检查，零开销抽象

---

## 📦 快速上手

```cpp
#include "GradualChanger.hpp"
#include <iostream>

int main() {
    int clock = 0;          // 外部时钟，可自己 ++ 也可按 dt 累加
    float hp = 100.0f;      // 要被渐变的变量

    // 3 秒内把 hp 从 100 → 0（假设 60 fps）
    new GradualChanger<float>(hp, 0.0f, clock, 180);

    for (; clock < 200; ++clock) {
        GradualChangerBase::update();   // 驱动所有渐变器
        if (clock % 20 == 0)
            std::cout << "t=" << clock << "  hp=" << hp << '\n';
    }
}
```

输出示例  
```
t=0  hp=100
t=20  hp=88.88
t=40  hp=77.77
...
t=180  hp=0
```

---

## 🧩 自定义类型

只要你的类型支持 `+` 与 `*`（乘 scalar），就能渐变：

```cpp
struct Vec3 { float x,y,z; };

template<>
struct GradualTraits<Vec3> {
    static Vec3 step(const Vec3& a, const Vec3& b, int steps) {
        float k = 1.0f / steps;
        return { (b.x-a.x)*k, (b.y-a.y)*k, (b.z-a.z)*k };
    }
};

Vec3 pos{0,0,0};
new GradualChanger<Vec3>(pos, Vec3{100,200,0}, clock, 60);
```

---

## 🧪 API 速查

| 接口 | 说明 |
|---|---|
| `GradualChanger<T>(ref, target, clock, duration)` | 创建渐变任务，duration=帧数 |
| `GradualChangerBase::update()` | 每帧调用一次，更新+清理 |
| `makeGradualChanger(variantRef, target, clock, duration)` | 自动推导类型的工厂函数 |

---

## ⚙️ 编译要求

- C++17 及以上  

---

## 📄 许可证

MIT © 2025 Library200  
欢迎 issue / PR / star ⭐
