// check.sh: expected-warning (the bugs are intentional)
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct NoNoexcept {
    std::string s;
    NoNoexcept(std::string x) : s{std::move(x)} {}
    NoNoexcept(const NoNoexcept& o) : s{o.s} { std::cout << "C"; }
    NoNoexcept(NoNoexcept&& o) : s{std::move(o.s)} { std::cout << "M"; } // BUG 4: not noexcept
};

std::string make_greeting() {
    std::string g = "hello";
    return std::move(g); // BUG 3: pessimizing move — disables NRVO (clang warns: -Wpessimizing-move)
}

int main() {
    // BUG 1: use after move. `a` is valid but unspecified; don't rely on its value.
    std::string a = "data";
    std::string b = std::move(a);
    std::cout << "1) a after move: '" << a << "' (don't read this in real code)\n";

    // BUG 2: std::move on const -> silent COPY, the "move" does nothing
    const std::string c = "constant";
    std::string d = std::move(c);
    std::cout << "2) c still holds: '" << c << "' -> a copy happened\n";

    std::cout << "3) " << make_greeting() << '\n';

    // BUG 4: vector growth copies (C) instead of moving (M) when move isn't noexcept
    std::cout << "4) growth: ";
    std::vector<NoNoexcept> v;
    for (int i = 0; i < 5; ++i) v.emplace_back("x");
    std::cout << "  <- C means copies; add noexcept to the move ctor to get M\n";
    (void)b;
    (void)d;
}
