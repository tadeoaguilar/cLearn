#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

template <typename T, typename... Ts>
T min_of(T first, Ts... rest) {
    static_assert((std::is_same_v<T, Ts> && ...), "min_of: all arguments must have the same type");
    T result = first;
    ((result = rest < result ? rest : result), ...); // fold over the comma operator
    return result;
}

template <typename... Args>
std::string make_string(const Args&... args) {
    std::ostringstream os;
    (os << ... << args);
    return os.str();
}

template <typename Pred, typename... Args>
std::size_t count_if_args(Pred pred, const Args&... args) {
    return (std::size_t{0} + ... + (pred(args) ? 1u : 0u));
}

int main() {
    std::cout << min_of(5, 3, 9, 1, 7) << '\n';
    std::cout << min_of(std::string{"pear"}, std::string{"apple"}, std::string{"fig"}) << '\n';
    // min_of(1, 2.0); // static_assert fails with our message

    std::string s = make_string("id=", 42, " ratio=", 0.75, " ok=", true);
    std::cout << s << '\n';

    auto positive = [](auto x) { return x > 0; };
    std::cout << count_if_args(positive, 1, -2, 3.5, -4.0, 5) << " positive args\n";
}
