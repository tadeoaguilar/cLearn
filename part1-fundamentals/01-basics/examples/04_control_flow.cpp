// if / else, switch, loops, break/continue, init-statements.
#include <iostream>
#include <string>
#include <vector>

int main() {
    // if / else if / else
    for (int n : {-5, 0, 7}) {
        if (n > 0) {
            std::cout << n << " is positive\n";
        } else if (n < 0) {
            std::cout << n << " is negative\n";
        } else {
            std::cout << n << " is zero\n";
        }
    }

    // switch with fallthrough made explicit
    for (char grade : {'A', 'B', 'C', 'F'}) {
        switch (grade) {
            case 'A':
            case 'B':
                std::cout << grade << ": great\n";
                break;
            case 'C':
                std::cout << grade << ": ok\n";
                break;
            default:
                std::cout << grade << ": study more\n";
        }
    }

    // classic for, with continue / break
    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0) continue; // skip evens
        if (i > 7) break;         // stop early
        std::cout << i << ' ';
    }
    std::cout << '\n';

    // while and do-while
    int countdown = 3;
    while (countdown > 0) std::cout << countdown-- << "... ";
    std::cout << "liftoff!\n";

    int tries = 0;
    do { ++tries; } while (tries < 0); // body runs once even though cond is false
    std::cout << "do-while ran " << tries << " time(s)\n";

    // range-based for over a container
    std::vector<std::string> names{"Ada", "Bjarne", "Grace"};
    for (const auto& name : names) std::cout << "Hi " << name << '\n';

    // if with init-statement (C++17): `len` only lives inside the if/else
    if (auto len = names[1].size(); len > 5) {
        std::cout << names[1] << " has a long name (" << len << ")\n";
    }

    // nested loops: multiplication table
    for (int r = 1; r <= 3; ++r) {
        for (int c = 1; c <= 3; ++c) std::cout << r * c << '\t';
        std::cout << '\n';
    }
}
