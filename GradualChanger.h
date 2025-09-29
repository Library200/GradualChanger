#pragma once
#include <variant>
#include <type_traits>
#include <cmath>

// 可渐变的内置类型列表，用户可以往后面追加自己的类型
using GradualValue = std::variant<int, unsigned, long long,
    float, double, bool>;

//---------------- 工具：从 std::variant 取出对应类型 ----------------//
namespace detail {
    template<class T, class... Ts>
    struct index_of;
    template<class T, class... Ts>
    struct index_of<T, T, Ts...> : std::integral_constant<size_t, 0> {};
    template<class T, class U, class... Ts>
    struct index_of<T, U, Ts...> :
        std::integral_constant<size_t, 1 + index_of<T, Ts...>::value> {
    };
} // namespace detail

//---------------- 步长计算器（用户可特化） ----------------//
template<class T>
struct GradualTraits {
    // 默认用线性插值： (target - start) / steps
    static T step(const T& start, const T& target, int steps) {
        return (target - start) / double(steps);
    }
    // 当类型不能除法时，用户可特化此函数
};

// 对 bool 做特化：直接跳变
template<>
struct GradualTraits<bool> {
    static bool step(bool start, bool target, int) { return target; }
};

//---------------- 真正的渐变器 ----------------//
class GradualChangerBase {
public:
    struct Node {
        Node* next = nullptr;
        bool  dead = false;          // 死亡标记
        virtual ~Node() = default;
        virtual void tick() = 0;
    };
    static inline Node* head_ = nullptr;   // 全局链表头
public:
    // 每帧调用一次，驱动所有渐变器
    static void update() {
        Node* cur = head_, * prev = nullptr;
        while (cur) {
            Node* nxt = cur->next;
            if (cur->dead) {              // 已经死了，直接摘
                (prev ? prev->next : head_) = nxt;
                delete cur;
            }
            else {
                cur->tick();              // 活的才调用
                prev = cur;
            }
            cur = nxt;
        }
    }
};

template<class T>
class GradualChanger : public GradualChangerBase::Node {
    static_assert(std::is_same_v<T, std::decay_t<T>>);
    T& ref_;      // 被渐变的外部变量
    const T       target_;   // 目标值
    const T       step_;     // 每步增量（构造时算好）
    int           left_;     // 剩余步数
    const int& clock_;    // 外部时钟引用
    const int     startClk_; // 构造时的时钟值
    int prevClk_; // 上一次tick时clock_的值

    // 把 this 挂到全局链表
    void link() {
        this->next = GradualChangerBase::head_;
        GradualChangerBase::head_ = this;
    }
public:
    // 构造时一次性算好步长
    GradualChanger(T& ref, T target, const int& clock, int duration)
        : ref_(ref), target_(target),
        step_(GradualTraits<T>::step(ref, target, duration)),
        left_(duration),
        clock_(clock), startClk_(clock), prevClk_(clock_)
    {
        link();
    }

    // 每帧回调
    void tick() override {
        if (dead || clock_==prevClk_) return;
        int elapsed = clock_ - startClk_;
        if (elapsed <= 0 || left_<=0) {   // 时间到
            //ref_ = target_;
            this->dead = true;  // 只打标记，不改动 next
            return;
        }
        ref_ += step_ * (clock_-prevClk_);
        left_ -= 1;
        prevClk_ = clock_;
    }
};

//---------------- 便捷工厂：支持 std::variant 自动分发 ----------------//
template<class... Ts>
GradualChangerBase::Node* makeGradualChanger(std::variant<Ts...>& ref,
    std::variant<Ts...> target,
    const int& clock,
    int duration)
{
    return std::visit([&](auto& v) -> GradualChangerBase::Node* {
        using T = std::decay_t<decltype(v)>;
        return new GradualChanger<T>(v,
            std::get<T>(target),
            clock, duration);
        }, ref);
}
