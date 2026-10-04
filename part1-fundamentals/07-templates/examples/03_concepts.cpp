#include <concepts>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

// Constrain with a standard concept
template <std::integral T>
T gcd(T a, T b) {
    while (b != 0) {
        T t = a % b;
        a = b;
        b = t;
    }
    return a;
}

// Define our own concept: "something with area() and name()"
template <typename T>
concept Shape = requires(const T& s) {
    { s.area() } -> std::convertible_to<double>;
    { s.name() } -> std::convertible_to<std::string>;
};

struct Circle {
    double r;
    double area() const { return std::numbers::pi * r * r; }
    std::string name() const { return "circle"; }
};
struct Square {
    double side;
    double area() const { return side * side; }
    std::string name() const { return "square"; }
};
struct NotAShape {
    int x;
};

// Note: no inheritance needed. Any type with the right "shape" works.
void describe(const Shape auto& s) { std::cout << s.name() << " with area " << s.area() << '\n'; }

// Overloading by concept: the most constrained viable overload wins.
// (The parameter forms must match, all `const auto&` here; mixing by-value
// and by-reference would make the calls ambiguous.)
void show(const std::integral auto& x) { std::cout << "integer " << x << '\n'; }
void show(const std::floating_point auto& x) { std::cout << "floating " << x << '\n'; }
void show(const auto& x) { std::cout << "something else: " << x << '\n'; }

// A concept combining others + a requires clause on a container
template <typename C>
concept NumericRange = requires(const C& c) {
    c.begin();
    c.end();
    requires std::is_arithmetic_v<typename C::value_type>;
};

double average(const NumericRange auto& c) {
    double total = 0;
    std::size_t n = 0;
    for (auto x : c) {
        total += static_cast<double>(x);
        ++n;
    }
    return n ? total / static_cast<double>(n) : 0.0;
}

int main() {
    std::cout << "gcd(48, 18) = " << gcd(48, 18) << '\n';
    // gcd(4.5, 1.5); // error: 'double' does not satisfy 'integral' -- a clear message!

    describe(Circle{1});
    describe(Square{2});
    // describe(NotAShape{1}); // error: constraints not satisfied: no area()
    static_assert(Shape<Circle> && !Shape<NotAShape>);

    show(42);
    show(3.14);
    show("text");

    std::cout << "avg = " << average(std::vector<int>{1, 2, 3, 4}) << '\n';
}
