#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

class AppError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error; // inherit constructors
    virtual int http_status() const { return 500; }
};
class ValidationError : public AppError {
public:
    using AppError::AppError;
    int http_status() const override { return 400; }
};
class NotFoundError : public AppError {
public:
    using AppError::AppError;
    int http_status() const override { return 404; }
};
class ConflictError : public AppError {
public:
    using AppError::AppError;
    int http_status() const override { return 409; }
};

std::map<std::string, double> accounts{{"alice", 100}, {"bob", 20}, {"frozen", 500}};

void transfer(const std::string& from, const std::string& to, double amount) {
    if (amount <= 0) throw ValidationError("amount must be positive");
    if (from == to) throw ValidationError("cannot transfer to the same account");
    auto src = accounts.find(from);
    if (src == accounts.end()) throw NotFoundError("account '" + from + "' not found");
    auto dst = accounts.find(to);
    if (dst == accounts.end()) throw NotFoundError("account '" + to + "' not found");
    if (from == "frozen") throw std::logic_error("frozen accounts not implemented"); // a "bug"
    if (src->second < amount) throw ConflictError("insufficient funds in '" + from + "'");
    src->second -= amount;
    dst->second += amount;
}

// The boundary: translate any exception into a status + message
void handle_request(const std::string& from, const std::string& to, double amount) {
    try {
        transfer(from, to, amount);
        std::cout << "HTTP 200: transferred " << amount << " from " << from << " to " << to << '\n';
    } catch (const AppError& e) {
        std::cout << "HTTP " << e.http_status() << ": " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cout << "HTTP 500: internal error (" << e.what() << ")\n"; // log details, hide from client
    }
}

int main() {
    handle_request("alice", "bob", 30);
    handle_request("alice", "bob", -5);
    handle_request("alice", "carol", 10);
    handle_request("bob", "alice", 1000);
    handle_request("frozen", "alice", 1);
    std::cout << "alice=" << accounts["alice"] << " bob=" << accounts["bob"] << '\n';
}
