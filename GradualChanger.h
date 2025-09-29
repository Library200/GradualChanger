#pragma once
#include <variant>
#include <type_traits>
#include <cmath>

// �ɽ�������������б��û�����������׷���Լ�������
using GradualValue = std::variant<int, unsigned, long long,
    float, double, bool>;

//---------------- ���ߣ��� std::variant ȡ����Ӧ���� ----------------//
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

//---------------- �������������û����ػ��� ----------------//
template<class T>
struct GradualTraits {
    // Ĭ�������Բ�ֵ�� (target - start) / steps
    static T step(const T& start, const T& target, int steps) {
        return (target - start) / double(steps);
    }
    // �����Ͳ��ܳ���ʱ���û����ػ��˺���
};

// �� bool ���ػ���ֱ������
template<>
struct GradualTraits<bool> {
    static bool step(bool start, bool target, int) { return target; }
};

//---------------- �����Ľ����� ----------------//
class GradualChangerBase {
public:
    struct Node {
        Node* next = nullptr;
        bool  dead = false;          // �������
        virtual ~Node() = default;
        virtual void tick() = 0;
    };
    static inline Node* head_ = nullptr;   // ȫ������ͷ
public:
    // ÿ֡����һ�Σ��������н�����
    static void update() {
        Node* cur = head_, * prev = nullptr;
        while (cur) {
            Node* nxt = cur->next;
            if (cur->dead) {              // �Ѿ����ˣ�ֱ��ժ
                (prev ? prev->next : head_) = nxt;
                delete cur;
            }
            else {
                cur->tick();              // ��Ĳŵ���
                prev = cur;
            }
            cur = nxt;
        }
    }
};

template<class T>
class GradualChanger : public GradualChangerBase::Node {
    static_assert(std::is_same_v<T, std::decay_t<T>>);
    T& ref_;      // ��������ⲿ����
    const T       target_;   // Ŀ��ֵ
    const T       step_;     // ÿ������������ʱ��ã�
    int           left_;     // ʣ�ಽ��
    const int& clock_;    // �ⲿʱ������
    const int     startClk_; // ����ʱ��ʱ��ֵ
    int prevClk_; // ��һ��tickʱclock_��ֵ

    // �� this �ҵ�ȫ������
    void link() {
        this->next = GradualChangerBase::head_;
        GradualChangerBase::head_ = this;
    }
public:
    // ����ʱһ������ò���
    GradualChanger(T& ref, T target, const int& clock, int duration)
        : ref_(ref), target_(target),
        step_(GradualTraits<T>::step(ref, target, duration)),
        left_(duration),
        clock_(clock), startClk_(clock), prevClk_(clock_)
    {
        link();
    }

    // ÿ֡�ص�
    void tick() override {
        if (dead || clock_==prevClk_) return;
        int elapsed = clock_ - startClk_;
        if (elapsed <= 0 || left_<=0) {   // ʱ�䵽
            //ref_ = target_;
            this->dead = true;  // ֻ���ǣ����Ķ� next
            return;
        }
        ref_ += step_ * (clock_-prevClk_);
        left_ -= 1;
        prevClk_ = clock_;
    }
};

//---------------- ��ݹ�����֧�� std::variant �Զ��ַ� ----------------//
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
