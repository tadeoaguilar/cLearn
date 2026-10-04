#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {
    std::vector<std::string> inputs{"12 + 30", "7 / 0", "9 * 9", "5 ^ 2", "10 - 2.5", "abc + 1"};

    for (const auto& expr : inputs) {
        std::istringstream in(expr); // treat a string like an input stream
        double a{}, b{};
        char op{};
        if (!(in >> a >> op >> b)) {
            std::cout << expr << "  => error: could not parse\n";
            continue;
        }

        double result{};
        bool ok = true;
        switch (op) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/':
                if (b == 0.0) {
                    std::cout << expr << "  => error: division by zero\n";
                    ok = false;
                } else {
                    result = a / b;
                }
                break;
            default:
                std::cout << expr << "  => error: unknown operator '" << op << "'\n";
                ok = false;
        }
        if (ok) std::cout << expr << "  => " << result << '\n';
    }
}
