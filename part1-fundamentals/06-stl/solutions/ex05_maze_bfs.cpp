#include <array>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <string>
#include <utility>
#include <vector>

using Pos = std::pair<int, int>; // (row, col); std::pair already has operator<

std::optional<Pos> find(const std::vector<std::string>& maze, char target) {
    for (int r = 0; r < static_cast<int>(maze.size()); ++r)
        for (int c = 0; c < static_cast<int>(maze[r].size()); ++c)
            if (maze[r][c] == target) return Pos{r, c};
    return std::nullopt;
}

int main() {
    std::vector<std::string> maze{
        "##########",
        "#S...#...#",
        "#.##.#.#.#",
        "#.#..#.#.#",
        "#.#.##.#.#",
        "#...#..#E#",
        "###...##.#",
        "##########",
    };

    auto start = find(maze, 'S');
    auto end = find(maze, 'E');
    if (!start || !end) return 1;

    std::queue<Pos> frontier;
    std::map<Pos, Pos> came_from; // also serves as the "visited" set
    frontier.push(*start);
    came_from[*start] = *start;

    constexpr std::array<Pos, 4> dirs{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
    while (!frontier.empty()) {
        auto cur = frontier.front();
        frontier.pop();
        if (cur == *end) break;
        for (auto [dr, dc] : dirs) {
            Pos next{cur.first + dr, cur.second + dc};
            if (maze[next.first][next.second] == '#' || came_from.contains(next)) continue;
            came_from[next] = cur;
            frontier.push(next);
        }
    }

    if (!came_from.contains(*end)) {
        std::cout << "no path\n";
        return 0;
    }

    int steps = 0;
    for (Pos p = came_from[*end]; p != *start; p = came_from[p]) {
        maze[p.first][p.second] = '*';
        ++steps;
    }
    for (const auto& row : maze) std::cout << row << '\n';
    std::cout << "shortest path: " << steps + 1 << " steps\n";
}
