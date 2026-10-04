#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

struct Person {
    std::string name;
    int age;
    std::string city;
};

int main() {
    std::vector<Person> people{
        {"Ada", 36, "London"}, {"Linus", 54, "Portland"}, {"Grace", 85, "New York"},
        {"Bjarne", 73, "New York"}, {"Margaret", 87, "Boston"},
    };

    // Projection: sort by a member without writing a comparator lambda
    std::ranges::sort(people, {}, &Person::age);
    for (const auto& p : people) std::cout << p.name << '(' << p.age << ") ";
    std::cout << '\n';

    std::ranges::sort(people, std::ranges::greater{}, &Person::name);
    std::cout << "by name desc: " << people.front().name << " ... " << people.back().name << '\n';

    auto it = std::ranges::find(people, "Grace", &Person::name);
    if (it != people.end()) std::cout << "found " << it->name << " in " << it->city << '\n';

    auto ny = std::ranges::count(people, "New York", &Person::city);
    std::cout << "people in New York: " << ny << '\n';

    auto oldest = std::ranges::max_element(people, {}, &Person::age);
    std::cout << "oldest: " << oldest->name << '\n';

    // Views: lazy pipelines (more in chapter 10)
    std::cout << "names of people over 60:\n";
    for (const auto& name : people | std::views::filter([](const Person& p) { return p.age > 60; })
                                   | std::views::transform(&Person::name)) {
        std::cout << "  " << name << '\n';
    }

    std::vector<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int x : nums | std::views::reverse | std::views::take(3)) std::cout << x << ' ';
    std::cout << '\n';
}
