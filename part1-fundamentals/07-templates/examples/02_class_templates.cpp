#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

template <typename T, std::size_t Capacity>
class FixedStack {
public:
    void push(const T& value) {
        if (size_ == Capacity) throw std::overflow_error("stack full");
        data_[size_++] = value;
    }
    T pop() {
        if (size_ == 0) throw std::underflow_error("stack empty");
        return data_[--size_];
    }
    const T& top() const { return data_.at(size_ - 1); }
    bool empty() const { return size_ == 0; }
    std::size_t size() const { return size_; }
    static constexpr std::size_t capacity() { return Capacity; }

    // A member function template: convert to a stack of another type
    template <typename U>
    FixedStack<U, Capacity> convert() const {
        FixedStack<U, Capacity> out;
        for (std::size_t i = 0; i < size_; ++i) out.push(static_cast<U>(data_[i]));
        return out;
    }

private:
    std::array<T, Capacity> data_{};
    std::size_t size_ = 0;
};

template <typename K, typename V>
struct Pair {
    K key;
    V value;
};
// Deduction guide is implicit for aggregates in C++20: Pair p{"a", 1} works

int main() {
    FixedStack<int, 4> s;
    for (int i = 1; i <= 4; ++i) s.push(i * 10);
    std::cout << "top=" << s.top() << " size=" << s.size() << " cap=" << s.capacity() << '\n';
    try {
        s.push(50);
    } catch (const std::overflow_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }

    auto ds = s.convert<double>();
    std::cout << "converted pop: " << ds.pop() / 3 << '\n';

    FixedStack<std::string, 2> names;
    names.push("Ada");
    std::cout << names.pop() << '\n';

    Pair p{std::string{"answer"}, 42}; // CTAD: Pair<std::string, int>
    std::cout << p.key << " = " << p.value << '\n';

    // Each instantiation is a distinct type:
    static_assert(!std::is_same_v<FixedStack<int, 4>, FixedStack<int, 5>>);
}
