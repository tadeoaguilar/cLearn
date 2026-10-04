#include <cstring>
#include <iostream>
#include <utility>

class String {
public:
    String(const char* s = "") : size_{std::strlen(s)}, data_{allocate(size_)} {
        std::memcpy(data_, s, size_ + 1);
    }

    ~String() { delete[] data_; }

    String(const String& o) : size_{o.size_}, data_{allocate(size_)} { std::memcpy(data_, o.data_, size_ + 1); }

    String& operator=(const String& o) {
        if (this != &o) {
            String tmp{o};          // may throw: do it before touching *this
            swap(tmp);              // strong guarantee
        }
        return *this;
    }

    String(String&& o) noexcept : size_{std::exchange(o.size_, 0)}, data_{std::exchange(o.data_, nullptr)} {}

    String& operator=(String&& o) noexcept {
        if (this != &o) {
            delete[] data_;
            size_ = std::exchange(o.size_, 0);
            data_ = std::exchange(o.data_, nullptr);
        }
        return *this;
    }

    String& operator+=(const String& o) {
        char* bigger = allocate(size_ + o.size_);
        if (data_) std::memcpy(bigger, data_, size_);
        std::memcpy(bigger + size_, o.c_str(), o.size_ + 1);
        delete[] data_;
        data_ = bigger;
        size_ += o.size_;
        return *this;
    }

    void swap(String& o) noexcept {
        std::swap(size_, o.size_);
        std::swap(data_, o.data_);
    }

    std::size_t size() const { return size_; }
    const char* c_str() const { return data_ ? data_ : ""; } // moved-from objects have null data

    friend std::ostream& operator<<(std::ostream& os, const String& s) { return os << s.c_str(); }

    static inline int allocations = 0;

private:
    static char* allocate(std::size_t n) {
        ++allocations;
        return new char[n + 1];
    }

    std::size_t size_;
    char* data_;
};

int main() {
    String a = "hello";
    String b = a;           // copy: +1 allocation
    b += " world";          // +1 allocation
    std::cout << a << " | " << b << " | allocations so far: " << String::allocations << '\n';

    int before = String::allocations;
    String c = std::move(b); // no allocation
    String d;                // +1 (empty string)
    d = std::move(c);        // no allocation
    std::cout << "d = " << d << ", b = '" << b << "', c = '" << c << "'\n";
    std::cout << "allocations during moves (incl. 1 for empty d): " << String::allocations - before << '\n';

    d = a; // copy assignment
    std::cout << "d after copy-assign: " << d << '\n';
}
