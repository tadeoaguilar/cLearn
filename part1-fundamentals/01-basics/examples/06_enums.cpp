// Scoped enumerations (enum class) and converting them to strings.
#include <iostream>
#include <string_view>

enum class Direction { North, East, South, West };

std::string_view to_string(Direction d) {
    switch (d) {
        case Direction::North: return "North";
        case Direction::East:  return "East";
        case Direction::South: return "South";
        case Direction::West:  return "West";
    }
    return "?"; // unreachable if all cases handled; -Wall warns if you miss one
}

Direction turn_right(Direction d) {
    // enum class does not convert to int implicitly; be explicit.
    int next = (static_cast<int>(d) + 1) % 4;
    return static_cast<Direction>(next);
}

int main() {
    Direction d = Direction::North;
    for (int i = 0; i < 5; ++i) {
        std::cout << to_string(d) << '\n';
        d = turn_right(d);
    }
}
