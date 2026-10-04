#include <cmath>
#include <compare>
#include <iostream>
#include <vector>

struct Vec2 {
    double x{};
    double y{};

    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(double s)      { x *= s;   y *= s;   return *this; }

    Vec2 operator-() const { return {-x, -y}; } // unary minus

    double length() const { return std::hypot(x, y); }

    // C++20: compiler writes ==, !=, <, <=, >, >= comparing members in order
    auto operator<=>(const Vec2&) const = default;

    // Non-member friends defined inline: symmetric binary operators
    friend Vec2 operator+(Vec2 a, const Vec2& b) { return a += b; }
    friend Vec2 operator-(Vec2 a, const Vec2& b) { return a -= b; }
    friend Vec2 operator*(Vec2 v, double s) { return v *= s; }
    friend Vec2 operator*(double s, Vec2 v) { return v *= s; } // so 2.0 * v also works

    friend std::ostream& operator<<(std::ostream& os, const Vec2& v) {
        return os << '(' << v.x << ", " << v.y << ')';
    }
};

// A class with a subscript and call operator
class Polynomial {
public:
    Polynomial(std::initializer_list<double> coeffs) : c_(coeffs) {}
    double& operator[](std::size_t i) { return c_.at(i); }
    double operator()(double x) const { // evaluate with Horner's method
        double result = 0;
        for (auto it = c_.rbegin(); it != c_.rend(); ++it) result = result * x + *it;
        return result;
    }

private:
    std::vector<double> c_; // c_[i] is the coefficient of x^i
};

int main() {
    Vec2 a{1, 2}, b{3, 4};
    std::cout << "a + b   = " << a + b << '\n';
    std::cout << "b - a   = " << b - a << '\n';
    std::cout << "2 * a   = " << 2.0 * a << '\n';
    std::cout << "-a      = " << -a << '\n';
    std::cout << "|b|     = " << b.length() << '\n';
    std::cout << std::boolalpha;
    std::cout << "a == b  = " << (a == b) << '\n';
    std::cout << "a < b   = " << (a < b) << '\n';

    Polynomial p{1, 0, 2}; // 1 + 0x + 2x^2
    std::cout << "p(3)    = " << p(3) << '\n';
    p[1] = 5;              // 1 + 5x + 2x^2
    std::cout << "p(3)    = " << p(3) << '\n';
}
