// A dynamic int buffer that manages its own memory. In real code you'd just
// use std::vector<int> (Rule of Zero); this shows what vector does for you.
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <utility>

class IntBuffer {
public:
    explicit IntBuffer(std::size_t n = 0) : size_{n}, data_{n ? new int[n]{} : nullptr} {}

    ~IntBuffer() { delete[] data_; }                                   // 1. destructor

    IntBuffer(const IntBuffer& other)                                  // 2. copy ctor: DEEP copy
        : size_{other.size_}, data_{size_ ? new int[size_] : nullptr} {
        std::copy(other.data_, other.data_ + size_, data_);
    }

    IntBuffer(IntBuffer&& other) noexcept                              // 4. move ctor: steal
        : size_{std::exchange(other.size_, 0)}, data_{std::exchange(other.data_, nullptr)} {}

    // 3 + 5: one assignment operator covers copy AND move (copy-and-swap).
    // `other` is constructed by copy or by move depending on the argument.
    IntBuffer& operator=(IntBuffer other) noexcept {
        swap(*this, other);
        return *this;
    } // old contents destroyed with `other`

    friend void swap(IntBuffer& a, IntBuffer& b) noexcept {
        using std::swap;
        swap(a.size_, b.size_);
        swap(a.data_, b.data_);
    }

    int& operator[](std::size_t i) { return data_[i]; }
    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    int* data_;
};

int main() {
    IntBuffer a{3};
    a[0] = 1; a[1] = 2; a[2] = 3;

    IntBuffer b = a;          // deep copy
    b[0] = 100;
    std::cout << "a[0]=" << a[0] << " b[0]=" << b[0] << " (independent)\n";

    IntBuffer c = std::move(a); // move: c steals a's array, no allocation
    std::cout << "c.size()=" << c.size() << " a.size()=" << a.size() << " (moved-from)\n";

    b = c;                    // copy-assign
    c = IntBuffer{5};         // move-assign from a temporary
    std::cout << "b.size()=" << b.size() << " c.size()=" << c.size() << '\n';
    // All memory released correctly: try running with -fsanitize=address
}
