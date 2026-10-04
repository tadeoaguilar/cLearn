// The many ways to initialize variables, auto, const and constexpr.
#include <iostream>
#include <string>
#include <vector>

constexpr int square(int x) { return x * x; } // can run at compile time

int main() {
    int a = 1;        // copy-initialization
    int b(2);         // direct-initialization
    int c{3};         // brace-initialization (preferred: no narrowing)
    int d{};          // zero
    // int e{3.7};    // ERROR: narrowing conversion from double to int
    int f = static_cast<int>(3.7); // explicit, intentional truncation → 3

    auto g = 42;            // int
    auto h = 42u;           // unsigned int
    auto i = 42.0f;         // float
    auto s = std::string{"text"}; // std::string (a plain "text" would be const char*)

    const int limit = 100;          // runtime constant, can't be modified
    constexpr int table = square(8); // computed by the compiler
    static_assert(table == 64, "checked at compile time");

    std::vector<int> v{1, 2, 3};     // braces → list of elements
    std::vector<int> w(3, 7);        // parens → 3 copies of 7: {7,7,7}

    std::cout << a << ' ' << b << ' ' << c << ' ' << d << ' ' << f << '\n';
    std::cout << g << ' ' << h << ' ' << i << ' ' << s << '\n';
    std::cout << "limit=" << limit << " table=" << table << '\n';
    std::cout << "v.size()=" << v.size() << " w.size()=" << w.size() << " w[0]=" << w[0] << '\n';
}
