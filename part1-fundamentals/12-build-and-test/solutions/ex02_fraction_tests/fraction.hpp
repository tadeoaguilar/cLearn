// Fraction from chapter 4, moved into a header so it can be tested.
#pragma once

#include <compare>
#include <numeric>
#include <ostream>
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
