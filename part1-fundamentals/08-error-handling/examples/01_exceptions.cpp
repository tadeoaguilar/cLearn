#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class InsufficientFunds : public std::runtime_error {
public:
    InsufficientFunds(double requested, double available)
        : std::runtime_error("insufficient funds: requested " + std::to_string(requested) + ", available " +
                             std::to_string(available)),
          requested_{requested} {}
    double requested() const { return requested_; }

private:
    double requested_;
};

struct Guard { // shows that destructors run during unwinding
    std::string name;
    ~Guard() { std::cout << "  ~Guard(" << name << ")\n"; }
};

void withdraw(double balance, double amount) {
    Guard g{"withdraw"};
    if (amount <= 0) throw std::invalid_argument("amount must be positive");
    if (amount > balance) throw InsufficientFunds(amount, balance);
    std::cout << "  withdrew " << amount << '\n';
}

void process(double amount) {
    Guard g{"process"};
    try {
        withdraw(100, amount);
    } catch (const InsufficientFunds& e) {
        std::cout << "  process: logging and rethrowing\n";
        throw; // rethrow the SAME exception object (keeps its dynamic type)
    }
}

int main() {
    for (double amount : {50.0, -1.0, 500.0}) {
        std::cout << "amount " << amount << ":\n";
        try {
            process(amount);
        } catch (const InsufficientFunds& e) { // most specific first
            std::cout << "  caught InsufficientFunds: " << e.what() << '\n';
        } catch (const std::logic_error& e) {   // invalid_argument derives from logic_error
            std::cout << "  caught logic_error: " << e.what() << '\n';
        } catch (const std::exception& e) {
            std::cout << "  caught exception: " << e.what() << '\n';
        }
    }

    // Standard library functions throw too
    std::vector<int> v{1, 2, 3};
    try {
        std::cout << v.at(10);
    } catch (const std::out_of_range& e) {
        std::cout << "vector::at -> out_of_range\n";
    }
    try {
        std::stoi("not a number");
    } catch (const std::invalid_argument&) {
        std::cout << "stoi -> invalid_argument\n";
    }
}
