// A header-only "module". Everything defined here must be inline (or constexpr,
// or a template) so that including it from several .cpp files is legal.
#pragma once

#include <cmath>
#include <numbers>

namespace geo {

struct Point {
    double x{};
    double y{};
};

inline double distance(Point a, Point b) {
    return std::hypot(b.x - a.x, b.y - a.y);
}

constexpr double circle_area(double r) { return std::numbers::pi * r * r; }

constexpr Point midpoint(Point a, Point b) { return {(a.x + b.x) / 2, (a.y + b.y) / 2}; }

} // namespace geo
