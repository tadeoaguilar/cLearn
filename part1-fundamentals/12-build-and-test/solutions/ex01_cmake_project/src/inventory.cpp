#include "inventory/inventory.hpp"

#include <numeric>
#include <stdexcept>

namespace inv {

void Inventory::add(const std::string& item, int qty) {
    if (qty <= 0) throw std::invalid_argument("qty must be positive");
    stock_[item] += qty;
}

bool Inventory::remove(const std::string& item, int qty) {
    auto it = stock_.find(item);
    if (it == stock_.end() || it->second < qty) return false;
    it->second -= qty;
    if (it->second == 0) stock_.erase(it);
    return true;
}

int Inventory::quantity(const std::string& item) const {
    auto it = stock_.find(item);
    return it == stock_.end() ? 0 : it->second;
}

int Inventory::total_items() const {
    return std::accumulate(stock_.begin(), stock_.end(), 0, [](int sum, const auto& kv) { return sum + kv.second; });
}

} // namespace inv
