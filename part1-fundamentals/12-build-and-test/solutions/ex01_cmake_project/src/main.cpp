#include <iostream>

#include "inventory/inventory.hpp"

int main() {
    inv::Inventory store;
    store.add("apple", 10);
    store.add("pear", 4);
    std::cout << "remove 3 apples: " << std::boolalpha << store.remove("apple", 3) << '\n';
    std::cout << "remove 9 pears:  " << store.remove("pear", 9) << '\n';
    std::cout << "apples: " << store.quantity("apple") << ", total: " << store.total_items() << '\n';
}
