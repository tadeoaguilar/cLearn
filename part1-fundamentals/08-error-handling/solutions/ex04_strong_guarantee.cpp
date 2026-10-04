#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

struct Op {
    std::string item;
    int delta;
};

class Inventory {
public:
    void apply(const std::vector<Op>& ops) {
        auto staged = stock_; // 1. work on a copy (may throw bad_alloc — that's fine, nothing changed)
        for (const auto& op : ops) {
            int& qty = staged[op.item];
            if (qty + op.delta < 0)
                throw std::runtime_error("not enough '" + op.item + "' (have " + std::to_string(qty) +
                                         ", change " + std::to_string(op.delta) + ")");
            qty += op.delta;
        }
        stock_.swap(staged); // 2. commit: swap never throws
    }

    void print() const {
        for (const auto& [item, qty] : stock_) std::cout << "  " << item << ": " << qty << '\n';
    }

private:
    std::map<std::string, int> stock_;
};

int main() {
    Inventory inv;
    inv.apply({{"apple", 10}, {"bread", 5}});
    std::cout << "initial:\n";
    inv.print();

    try {
        inv.apply({{"apple", -3}, {"milk", 4}, {"bread", -9}}); // last op fails
    } catch (const std::exception& e) {
        std::cout << "batch failed: " << e.what() << '\n';
    }
    std::cout << "after failed batch (unchanged, no 'milk'):\n";
    inv.print();

    inv.apply({{"apple", -3}, {"milk", 4}});
    std::cout << "after good batch:\n";
    inv.print();
}
