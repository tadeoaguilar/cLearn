#include <iostream>
#include <memory>
#include <string>
#include <utility>

void target(const std::string& s) { std::cout << "  target(const&): " << s << '\n'; }
void target(std::string&& s)      { std::cout << "  target(&&):     " << s << '\n'; }

template <typename T>
void bad_wrapper(T&& arg) {
    target(arg); // `arg` has a name → it's an lvalue → always calls const& version
}

template <typename T>
void good_wrapper(T&& arg) {
    target(std::forward<T>(arg)); // restores the original value category
}

// A factory just like std::make_unique
template <typename T, typename... Args>
std::unique_ptr<T> make(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct Player {
    std::string name;
    int level;
    Player(std::string n, int l) : name{std::move(n)}, level{l} {}
};

int main() {
    std::string s = "lvalue";
    std::cout << "bad_wrapper:\n";
    bad_wrapper(s);
    bad_wrapper(std::string{"rvalue"}); // should be && but isn't

    std::cout << "good_wrapper:\n";
    good_wrapper(s);
    good_wrapper(std::string{"rvalue"});

    auto p = make<Player>("Ada", 42);
    std::cout << p->name << " lvl " << p->level << '\n';
}
