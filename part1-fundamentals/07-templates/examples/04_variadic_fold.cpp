#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

// Print any number of arguments of any printable types
template <typename... Args>
void log(const Args&... args) {
    std::cout << "[log] ";
    (std::cout << ... << args) << '\n'; // binary left fold over <<
}

template <typename... Ts>
auto sum(Ts... xs) {
    return (xs + ... + 0); // unary right fold with init 0 (works for empty packs too)
}

template <typename First, typename... Rest>
constexpr bool all_same_v = (std::is_same_v<First, Rest> && ...);

// Fold with a separator using the comma operator
template <typename... Args>
void print_csv(const Args&... args) {
    std::size_t i = 0;
    ((std::cout << (i++ ? ", " : "") << args), ...);
    std::cout << '\n';
}

// Count arguments
template <typename... Ts>
constexpr std::size_t count_args(const Ts&...) { return sizeof...(Ts); }

// Print a tuple using std::apply (unpacks tuple into a pack)
template <typename Tuple>
void print_tuple(const Tuple& t) {
    std::apply([](const auto&... elems) { print_csv(elems...); }, t);
}

int main() {
    log("x=", 3, ", y=", 4.5, ", name=", std::string{"Ada"});
    std::cout << "sum = " << sum(1, 2, 3, 4, 5) << '\n';
    std::cout << "sum() = " << sum() << '\n';
    std::cout << std::boolalpha << "all_same<int,int,int> = " << all_same_v<int, int, int> << '\n';
    std::cout << "all_same<int,double> = " << all_same_v<int, double> << '\n';
    print_csv("apple", 3, 2.5, 'x');
    std::cout << "count_args = " << count_args(1, "two", 3.0) << '\n';
    print_tuple(std::make_tuple(1, "tuple", 3.14));
}
