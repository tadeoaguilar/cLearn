// check.sh: expected-warning (demonstrates -Wunused-but-set-variable)
#include <iostream>
#include <string>
#include <vector>

int main() {
    int x = 10;
    int& r = x; // r is another name for x
    r += 5;
    std::cout << "x = " << x << ", &x == &r ? " << std::boolalpha << (&x == &r) << '\n';

    // A reference can't be re-seated: this ASSIGNS y's value to x
    int y = 99;
    r = y;
    std::cout << "after r = y: x = " << x << '\n';

    // const references can bind to temporaries (lifetime is extended)
    const std::string& greeting = std::string("hello") + " world";
    std::cout << greeting << '\n';

    // References in range-for: modify in place vs copy
    std::vector<int> v{1, 2, 3};
    for (int e : v) e *= 10;   // modifies copies: no effect on v (the compiler even warns:
                               // "variable 'e' set but not used" -- a hint something's off)
    for (int& e : v) e *= 10;  // modifies the elements
    for (const int& e : v) std::cout << e << ' ';
    std::cout << '\n';

    // Reference to an element of a container
    std::vector<std::string> names{"ada", "linus"};
    std::string& first = names[0];
    first[0] = 'A';
    std::cout << names[0] << '\n';
}
