#include <compare>
#include <iostream>
#include <numeric> // std::gcd
#include <stdexcept>

class Fraction {
public:
    Fraction(long long num = 0, long long den = 1) : num_{num}, den_{den} { normalize(); }

    long long num() const { return num_; }
    long long den() const { return den_; }
    double to_double() const { return static_cast<double>(num_) / static_cast<double>(den_); }

    Fraction& operator+=(const Fraction& o) { num_ = num_ * o.den_ + o.num_ * den_; den_ *= o.den_; normalize(); return *this; }
    Fraction& operator-=(const Fraction& o) { num_ = num_ * o.den_ - o.num_ * den_; den_ *= o.den_; normalize(); return *this; }
    Fraction& operator*=(const Fraction& o) { num_ *= o.num_; den_ *= o.den_; normalize(); return *this; }
    Fraction& operator/=(const Fraction& o) {
        if (o.num_ == 0) throw std::domain_error("division by zero fraction");
        num_ *= o.den_;
        den_ *= o.num_;
        normalize();
        return *this;
    }

    friend Fraction operator+(Fraction a, const Fraction& b) { return a += b; }
    friend Fraction operator-(Fraction a, const Fraction& b) { return a -= b; }
    friend Fraction operator*(Fraction a, const Fraction& b) { return a *= b; }
    friend Fraction operator/(Fraction a, const Fraction& b) { return a /= b; }

    // Because we're always normalized, member-wise equality is correct.
    bool operator==(const Fraction&) const = default;

    // Compare a/b vs c/d via a*d vs c*b (denominators are positive).
    std::strong_ordering operator<=>(const Fraction& o) const { return num_ * o.den_ <=> o.num_ * den_; }

    friend std::ostream& operator<<(std::ostream& os, const Fraction& f) {
        os << f.num_;
        if (f.den_ != 1) os << '/' << f.den_;
        return os;
    }

private:
    void normalize() {
        if (den_ == 0) throw std::invalid_argument("zero denominator");
        if (den_ < 0) { num_ = -num_; den_ = -den_; }
        long long g = std::gcd(num_, den_);
        if (g > 1) { num_ /= g; den_ /= g; }
    }

    long long num_;
    long long den_;
};

int main() {
    Fraction half{1, 2}, third{1, 3};
    std::cout << half << " + " << third << " = " << half + third << '\n';
    std::cout << half << " - " << third << " = " << half - third << '\n';
    std::cout << half << " * " << third << " = " << half * third << '\n';
    std::cout << half << " / " << third << " = " << half / third << '\n';
    std::cout << std::boolalpha;
    std::cout << "1/2 + 1/3 == 5/6 ? " << (half + third == Fraction{5, 6}) << '\n';
    std::cout << "2/4 == 1/2 ? " << (Fraction{2, 4} == half) << '\n';
    std::cout << "1/3 < 1/2 ? " << (third < half) << '\n';
    std::cout << "3/-6 normalizes to " << Fraction{3, -6} << '\n';
    std::cout << "4/2 prints as " << Fraction{4, 2} << " (" << Fraction{4, 2}.to_double() << ")\n";
    try {
        Fraction bad{1, 0};
    } catch (const std::invalid_argument& e) {
        std::cout << "error: " << e.what() << '\n';
    }
}
