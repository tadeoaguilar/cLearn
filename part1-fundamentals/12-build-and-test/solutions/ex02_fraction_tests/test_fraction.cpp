#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <sstream>

#include "fraction.hpp"

TEST_CASE("construction normalizes") {
    Fraction f{2, 4};
    CHECK(f.num() == 1);
    CHECK(f.den() == 2);

    SUBCASE("negative denominator moves the sign up") {
        Fraction g{3, -6};
        CHECK(g.num() == -1);
        CHECK(g.den() == 2);
    }
    SUBCASE("zero numerator") {
        Fraction z{0, 5};
        CHECK(z.num() == 0);
        CHECK(z.den() == 1);
    }
    SUBCASE("zero denominator throws") { CHECK_THROWS_AS(Fraction(1, 0), std::invalid_argument); }
}

TEST_CASE("arithmetic") {
    Fraction half{1, 2}, third{1, 3};
    CHECK(half + third == Fraction{5, 6});
    CHECK(half - third == Fraction{1, 6});
    CHECK(half * third == Fraction{1, 6});
    CHECK(half / third == Fraction{3, 2});
    CHECK_THROWS_AS(half / Fraction{0}, std::domain_error);
}

TEST_CASE("comparison") {
    CHECK(Fraction{1, 3} < Fraction{1, 2});
    CHECK(Fraction{-1, 2} < Fraction{1, 3});
    CHECK(Fraction{2, 4} == Fraction{1, 2});
    CHECK(Fraction{3, 4} >= Fraction{6, 8});
}

TEST_CASE("printing") {
    std::ostringstream os;
    os << Fraction{6, 4} << ' ' << Fraction{4, 2};
    CHECK(os.str() == "3/2 2");
}

TEST_CASE("to_double") { CHECK(Fraction{1, 4}.to_double() == doctest::Approx(0.25)); }
