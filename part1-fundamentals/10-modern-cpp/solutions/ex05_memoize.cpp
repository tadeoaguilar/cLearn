#include <iostream>
#include <map>
#include <memory>

template <typename R, typename Arg>
auto memoize(R (*f)(Arg)) {
    // shared_ptr: copies of the returned lambda share one cache
    auto cache = std::make_shared<std::map<Arg, R>>();
    return [f, cache](Arg a) -> R {
        if (auto it = cache->find(a); it != cache->end()) return it->second;
        R result = f(a);
        cache->emplace(a, result);
        return result;
    };
}

int g_calls = 0;
long long slow_square(int x) {
    ++g_calls;
    return static_cast<long long>(x) * x;
}

int main() {
    auto fast = memoize(slow_square);
    for (int x : {3, 4, 3, 3, 4, 5}) std::cout << x << "^2 = " << fast(x) << '\n';
    std::cout << "real function ran " << g_calls << " times (expected 3)\n";

    auto copy = fast; // shares the cache
    copy(5);
    std::cout << "after copy(5): still " << g_calls << " calls\n";
}
