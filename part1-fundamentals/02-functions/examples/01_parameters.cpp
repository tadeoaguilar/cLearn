// How parameter passing affects the caller's data and performance.
#include <iostream>
#include <string>
#include <vector>

void by_value(std::string s)          { s += " (changed by value)"; }
void by_reference(std::string& s)     { s += " (changed by reference)"; }
void by_const_ref(const std::string& s) {
    // s += "x";   // ERROR: s is const
    std::cout << "reading: " << s << '\n';
}
void by_pointer(std::string* s) {
    if (s == nullptr) { std::cout << "got nullptr, nothing to do\n"; return; }
    *s += " (changed by pointer)"; // * dereferences the pointer
}

// Returning by value is the idiomatic way to produce results.
std::vector<int> make_range(int from, int to) {
    std::vector<int> out;
    for (int i = from; i <= to; ++i) out.push_back(i);
    return out; // no copy: return value optimization / move
}

// Several results: return a struct
struct Division { int quotient; int remainder; };
Division divide(int a, int b) { return {a / b, a % b}; }

int main() {
    std::string text = "original";

    by_value(text);
    std::cout << text << '\n';      // original

    by_reference(text);
    std::cout << text << '\n';      // original (changed by reference)

    by_const_ref(text);

    by_pointer(&text);              // & takes the address
    by_pointer(nullptr);
    std::cout << text << '\n';

    auto nums = make_range(1, 5);
    for (int n : nums) std::cout << n << ' ';
    std::cout << '\n';

    auto [q, r] = divide(17, 5);    // structured binding
    std::cout << "17 / 5 = " << q << " remainder " << r << '\n';
}
