#include <iostream>
#include <stdexcept>
#include <string>

struct Noisy {
    std::string name;
    explicit Noisy(std::string n) : name{std::move(n)} { std::cout << "  + " << name << '\n'; }
    ~Noisy() { std::cout << "  - " << name << '\n'; }
};

void leaky(bool fail) {
    Noisy* p = new Noisy{"heap (raw)"};
    if (fail) {
        std::cout << "  early return: the delete below never runs → LEAK\n";
        return;
    }
    delete p;
}

void safe(bool fail) {
    Noisy local{"stack"}; // destroyed on ANY exit path
    if (fail) throw std::runtime_error("oops");
    std::cout << "  normal exit\n";
}

int counter() {
    static int calls = 0; // static storage: initialized once, survives between calls
    return ++calls;
}

int main() {
    std::cout << "leaky(false):\n"; leaky(false);
    std::cout << "leaky(true):\n";  leaky(true);

    std::cout << "safe(false):\n";  safe(false);
    std::cout << "safe(true):\n";
    try {
        safe(true);
    } catch (const std::exception& e) {
        std::cout << "  caught: " << e.what() << " (and 'stack' was still destroyed)\n";
    }

    counter(); counter();
    std::cout << "counter() called " << counter() << " times\n";
}
