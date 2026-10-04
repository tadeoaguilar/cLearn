#include <algorithm>
#include <format>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

struct Student {
    std::string name;
    std::vector<int> scores;
    double average() const {
        return scores.empty() ? 0.0 : std::accumulate(scores.begin(), scores.end(), 0.0) / static_cast<double>(scores.size());
    }
};

int main() {
    std::vector<Student> cls{
        {"Ada", {95, 88, 92}}, {"Bob", {55, 61, 48}}, {"Cy", {72, 80, 79}},
        {"Dee", {40, 65, 52}}, {"Eve", {88, 99, 94}},
    };

    // Rank by average, best first
    std::ranges::sort(cls, std::ranges::greater{}, &Student::average);
    std::cout << "Ranking:\n";
    for (std::size_t i = 0; i < cls.size(); ++i)
        std::cout << std::format("  {}. {:<4} {:6.2f}\n", i + 1, cls[i].name, cls[i].average());

    // Failing students: partition moves passing ones to the front
    auto failing_begin = std::partition(cls.begin(), cls.end(), [](const Student& s) { return s.average() >= 60; });
    std::cout << "Failing:";
    std::for_each(failing_begin, cls.end(), [](const Student& s) { std::cout << ' ' << s.name; });
    std::cout << '\n';

    // Top scorer per assignment
    for (std::size_t a = 0; a < cls.front().scores.size(); ++a) {
        auto best = std::ranges::max_element(cls, {}, [a](const Student& s) { return s.scores[a]; });
        std::cout << std::format("Assignment {} top: {} ({})\n", a + 1, best->name, best->scores[a]);
    }

    // Median of averages: nth_element is O(n), no full sort needed
    std::vector<double> avgs;
    std::ranges::transform(cls, std::back_inserter(avgs), &Student::average);
    auto mid = avgs.begin() + static_cast<std::ptrdiff_t>(avgs.size() / 2);
    std::nth_element(avgs.begin(), mid, avgs.end());
    std::cout << std::format("Median average: {:.2f}\n", *mid); // odd count → exact median
}
