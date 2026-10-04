// Using our own header and namespace.
#include <iostream>

#include "geometry.hpp" // quotes: search next to this file first; <> for system headers

int main() {
    geo::Point a{0, 0};
    geo::Point b{3, 4};

    std::cout << "distance = " << geo::distance(a, b) << '\n';

    constexpr auto area = geo::circle_area(2.0); // computed at compile time
    std::cout << "area r=2 = " << area << '\n';

    using geo::midpoint; // bring a single name into scope
    auto m = midpoint(a, b);
    std::cout << "midpoint = (" << m.x << ", " << m.y << ")\n";
}
