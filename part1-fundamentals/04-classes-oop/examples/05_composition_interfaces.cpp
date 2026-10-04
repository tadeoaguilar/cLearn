// Interfaces (pure abstract classes) + composition + dependency injection.
// This is the same design the CRUD API uses for its repositories.
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// "Interface": only pure virtuals + virtual dtor
class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(const std::string& msg) = 0;
};

class ConsoleLogger : public Logger {
public:
    void log(const std::string& msg) override { std::cout << "[console] " << msg << '\n'; }
};

class MemoryLogger : public Logger { // handy in tests
public:
    void log(const std::string& msg) override { lines.push_back(msg); }
    std::vector<std::string> lines;
};

// OrderService HAS-A Logger (composition). It depends on the interface, not on
// a concrete class, so we can swap the implementation.
class OrderService {
public:
    explicit OrderService(Logger& logger) : logger_{logger} {}

    void place_order(const std::string& item, int qty) {
        if (qty <= 0) {
            logger_.log("rejected order for " + item);
            return;
        }
        ++orders_;
        logger_.log("order #" + std::to_string(orders_) + ": " + std::to_string(qty) + " x " + item);
    }

private:
    Logger& logger_; // non-owning: the logger must outlive the service
    int orders_ = 0;
};

int main() {
    ConsoleLogger console;
    OrderService live{console};
    live.place_order("coffee", 2);
    live.place_order("tea", 0);

    MemoryLogger memory;
    OrderService tested{memory};
    tested.place_order("bagel", 1);
    std::cout << "memory logger captured " << memory.lines.size() << " line(s): " << memory.lines[0] << '\n';
}
