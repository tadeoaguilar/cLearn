// Three ways to write a "setter that stores its argument".
#include <iostream>
#include <string>
#include <utility>

struct Counted {
    static inline int copies = 0, moves = 0;
    std::string s;
    Counted(const char* x) : s{x} {}
    Counted(const Counted& o) : s{o.s} { ++copies; }
    Counted(Counted&& o) noexcept : s{std::move(o.s)} { ++moves; }
    Counted& operator=(const Counted& o) { s = o.s; ++copies; return *this; }
    Counted& operator=(Counted&& o) noexcept { s = std::move(o.s); ++moves; return *this; }
    static void reset() { copies = moves = 0; }
    static void report(const char* label) { std::cout << label << ": copies=" << copies << " moves=" << moves << '\n'; }
};

struct A { // const& only: always copies
    Counted v{""};
    void set(const Counted& x) { v = x; }
};
struct B { // two overloads: optimal, but 2^N overloads for N params
    Counted v{""};
    void set(const Counted& x) { v = x; }
    void set(Counted&& x) { v = std::move(x); }
};
struct C { // by value + move: one function, at most one extra move
    Counted v{""};
    void set(Counted x) { v = std::move(x); }
};

template <typename T>
void run(const char* label) {
    T obj;
    Counted lv{"lvalue"};
    Counted::reset();
    obj.set(lv);
    obj.set(Counted{"rvalue"});
    Counted::report(label);
}

int main() {
    run<A>("const&        ");
    run<B>("const& + &&   ");
    run<C>("by value+move ");
    std::cout << "Guideline: for sink params, by-value + std::move is simple and nearly optimal.\n";
}
