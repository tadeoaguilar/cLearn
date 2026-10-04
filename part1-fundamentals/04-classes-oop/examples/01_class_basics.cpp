#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

class BankAccount {
public:
    explicit BankAccount(std::string owner, double initial = 0.0)
        : owner_{std::move(owner)}, balance_{initial} {
        if (initial < 0) throw std::invalid_argument("initial balance must be >= 0");
    }

    void deposit(double amount) {
        if (amount <= 0) throw std::invalid_argument("deposit must be positive");
        balance_ += amount;
    }

    // Returns false instead of throwing: insufficient funds is an expected outcome
    bool withdraw(double amount) {
        if (amount <= 0) throw std::invalid_argument("withdrawal must be positive");
        if (amount > balance_) return false;
        balance_ -= amount;
        return true;
    }

    double balance() const { return balance_; }
    const std::string& owner() const { return owner_; } // return by const& avoids a copy

private:
    std::string owner_;
    double balance_{0.0}; // invariant: never negative
};

void print_statement(const BankAccount& acc) { // const& → only const methods allowed
    std::cout << acc.owner() << ": $" << acc.balance() << '\n';
    // acc.deposit(1); // ERROR: deposit is not const
}

int main() {
    BankAccount acc{"Ada", 100};
    acc.deposit(50);
    print_statement(acc);

    if (!acc.withdraw(500)) std::cout << "withdraw 500 refused: insufficient funds\n";
    acc.withdraw(30);
    print_statement(acc);

    // acc.balance_ = 1e9; // ERROR: private
    // BankAccount b = "Bob"; // ERROR thanks to explicit

    try {
        acc.deposit(-5);
    } catch (const std::invalid_argument& e) {
        std::cout << "error: " << e.what() << '\n';
    }
}
