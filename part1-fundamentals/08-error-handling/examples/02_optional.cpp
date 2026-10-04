#include <iostream>
#include <map>
#include <optional>
#include <string>

struct User {
    int id;
    std::string name;
    std::optional<std::string> email; // optional field: may be absent
};

std::map<int, User> db{
    {1, {1, "Ada", "ada@example.com"}},
    {2, {2, "Grace", std::nullopt}},
};

std::optional<User> find_user(int id) {
    if (auto it = db.find(id); it != db.end()) return it->second;
    return std::nullopt;
}

std::optional<std::string> email_domain(const User& u) {
    if (!u.email) return std::nullopt;
    auto at = u.email->find('@');
    if (at == std::string::npos) return std::nullopt;
    return u.email->substr(at + 1);
}

int main() {
    for (int id : {1, 2, 3}) {
        if (auto u = find_user(id)) {
            std::cout << id << ": " << u->name << ", email: " << u->email.value_or("<none>") << '\n';
        } else {
            std::cout << id << ": not found\n";
        }
    }

    // Monadic operations (C++23) avoid nested ifs:
    for (int id : {1, 2, 3}) {
        auto domain = find_user(id)
                          .and_then(email_domain) // optional<User> -> optional<string>
                          .transform([](const std::string& d) { return "@" + d; })
                          .value_or("(no domain)");
        std::cout << "user " << id << " domain: " << domain << '\n';
    }

    try {
        auto ghost = find_user(99).value(); // throws when empty
    } catch (const std::bad_optional_access& e) {
        std::cout << "value() on empty optional: " << e.what() << '\n';
    }
}
