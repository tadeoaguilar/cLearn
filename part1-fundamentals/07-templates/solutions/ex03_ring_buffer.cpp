#include <array>
#include <iostream>
#include <stdexcept>

template <typename T, std::size_t N>
class RingBuffer {
    static_assert(N > 0, "RingBuffer capacity must be positive");

public:
    void push(const T& v) {
        data_[head_] = v;
        head_ = (head_ + 1) % N;
        if (size_ < N) ++size_;
    }

    std::size_t size() const { return size_; }
    bool full() const { return size_ == N; }

    // 0 = oldest element
    const T& operator[](std::size_t i) const {
        if (i >= size_) throw std::out_of_range("RingBuffer index");
        std::size_t oldest = full() ? head_ : 0;
        return data_[(oldest + i) % N];
    }

    template <typename F>
    void for_each(F f) const {
        for (std::size_t i = 0; i < size_; ++i) f((*this)[i]);
    }

private:
    std::array<T, N> data_{};
    std::size_t head_ = 0; // next write position
    std::size_t size_ = 0;
};

int main() {
    RingBuffer<double, 3> window;
    for (double reading : {10.0, 12.0, 11.0, 30.0, 13.0, 12.0}) {
        window.push(reading);
        double sum = 0;
        window.for_each([&sum](double v) { sum += v; });
        std::cout << "reading " << reading << " -> moving avg over " << window.size() << " = "
                  << sum / static_cast<double>(window.size()) << '\n';
    }
    std::cout << "oldest kept: " << window[0] << ", newest: " << window[window.size() - 1] << '\n';
}
