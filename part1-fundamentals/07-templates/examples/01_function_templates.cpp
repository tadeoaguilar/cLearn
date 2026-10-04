#include <iostream>
#include <string>
#include <vector>

template <typename T>
T max_of(T a, T b) {
    return a > b ? a : b;
}

// Two independent type parameters; auto return type = whatever a + b yields
template <typename A, typename B>
auto add(A a, B b) {
    return a + b;
}

// Works with any container that supports range-for and whose elements support +=
template <typename Container>
auto sum(const Container& c) {
    typename Container::value_type total{}; // `typename` tells the compiler this is a type
    for (const auto& x : c) total += x;
    return total;
}

// Abbreviated function template (C++20): `auto` parameter = implicit template
void print_twice(const auto& x) { std::cout << x << ' ' << x << '\n'; }

int main() {
    std::cout << max_of(3, 7) << '\n';                 // max_of<int>
    std::cout << max_of(2.5, 1.5) << '\n';             // max_of<double>
    std::cout << max_of(std::string{"pear"}, std::string{"apple"}) << '\n';
    // max_of(3, 7.5);                                 // ERROR: T can't be both int and double
    std::cout << max_of<double>(3, 7.5) << '\n';       // explicit template argument

    std::cout << add(1, 2.5) << '\n';                  // double
    std::cout << add(std::string{"C++"}, "20") << '\n';

    std::cout << sum(std::vector<int>{1, 2, 3}) << '\n';
    std::cout << sum(std::vector<std::string>{"a", "b", "c"}) << '\n';

    print_twice(42);
    print_twice("hi");
}
