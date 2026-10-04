#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

struct Item {
    std::string name;
    int qty;
    Item(std::string n, int q) : name{std::move(n)}, qty{q} {}
};

template <typename T>
void print(const std::vector<T>& v) {
    for (const auto& x : v) std::cout << x << ' ';
    std::cout << "(size=" << v.size() << ", cap=" << v.capacity() << ")\n";
}

int main() {
    std::vector<int> v;
    std::cout << "growth: ";
    std::size_t last_cap = 0;
    for (int i = 0; i < 100; ++i) {
        v.push_back(i);
        if (v.capacity() != last_cap) {
            last_cap = v.capacity();
            std::cout << last_cap << ' '; // each change = reallocation + move of all elements
        }
    }
    std::cout << '\n';

    std::vector<int> r;
    r.reserve(100); // one allocation up front
    std::cout << "reserved cap: " << r.capacity() << " size: " << r.size() << '\n';

    std::vector<int> nums{5, 3, 8, 3, 1, 3};
    print(nums);
    nums.insert(nums.begin() + 1, 42);
    print(nums);
    nums.erase(nums.begin());
    print(nums);
    std::erase(nums, 3); // C++20: remove all 3s
    print(nums);
    nums.shrink_to_fit(); // request to release unused capacity
    print(nums);

    // emplace_back constructs in place from constructor arguments
    std::vector<Item> inventory;
    inventory.emplace_back("sword", 1);
    inventory.push_back(Item{"potion", 5}); // constructs a temporary, then moves it in
    for (const auto& [name, qty] : inventory) std::cout << name << " x" << qty << '\n';

    // 2D vector
    std::vector<std::vector<char>> grid(3, std::vector<char>(5, '.'));
    grid[1][2] = '@';
    for (const auto& row : grid) {
        for (char c : row) std::cout << c;
        std::cout << '\n';
    }
}
