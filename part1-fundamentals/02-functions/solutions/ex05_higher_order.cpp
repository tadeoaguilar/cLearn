#include <functional>
#include <iostream>
#include <vector>

std::vector<int> map_vec(const std::vector<int>& v, const std::function<int(int)>& f) {
    std::vector<int> out;
    out.reserve(v.size());
    for (int x : v) out.push_back(f(x));
    return out;
}

std::vector<int> filter_vec(const std::vector<int>& v, const std::function<bool(int)>& keep) {
    std::vector<int> out;
    for (int x : v)
        if (keep(x)) out.push_back(x);
    return out;
}

int reduce_vec(const std::vector<int>& v, int init, const std::function<int(int, int)>& combine) {
    int acc = init;
    for (int x : v) acc = combine(acc, x);
    return acc;
}

std::function<int(int)> compose(std::function<int(int)> f, std::function<int(int)> g) {
    // The returned lambda captures copies of f and g, so it stays valid after we return.
    return [f, g](int x) { return f(g(x)); };
}

int main() {
    std::vector<int> nums;
    for (int i = 1; i <= 10; ++i) nums.push_back(i);

    auto odds = filter_vec(nums, [](int x) { return x % 2 != 0; });
    auto squares = map_vec(odds, [](int x) { return x * x; });
    int total = reduce_vec(squares, 0, [](int a, int b) { return a + b; });
    std::cout << "sum of squares of odds in 1..10 = " << total << '\n'; // 1+9+25+49+81 = 165

    auto inc = [](int x) { return x + 1; };
    auto dbl = [](int x) { return x * 2; };
    auto inc_then_double = compose(dbl, inc);
    std::cout << "compose(dbl, inc)(5) = " << inc_then_double(5) << '\n'; // (5+1)*2 = 12
}
