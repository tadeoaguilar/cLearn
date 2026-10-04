#pragma once

#include <map>
#include <string>

namespace inv {

class Inventory {
public:
    void add(const std::string& item, int qty);
    // Returns false (and changes nothing) if there isn't enough stock.
    bool remove(const std::string& item, int qty);
    int quantity(const std::string& item) const;
    int total_items() const;

private:
    std::map<std::string, int> stock_;
};

} // namespace inv
