// std::string operations, std::format, and robust input parsing.
// Try: echo "42 hello world" | ./program
#include <format>
#include <iostream>
#include <string>

int main() {
    std::string first = "Ada";
    std::string last = "Lovelace";
    std::string full = first + " " + last;

    std::cout << full << " has " << full.size() << " characters\n";
    std::cout << "first char: " << full[0] << ", last char: " << full.back() << '\n';
    std::cout << "substr(4, 4): " << full.substr(4, 4) << '\n';

    if (auto pos = full.find("love"); pos == std::string::npos) {
        std::cout << "\"love\" not found (find is case-sensitive)\n";
    }
    std::cout << "starts with Ada? " << std::boolalpha << full.starts_with("Ada") << '\n';

    // Modify characters in place
    for (char& ch : full) {
        if (ch >= 'a' && ch <= 'z') ch = static_cast<char>(ch - 'a' + 'A');
    }
    std::cout << "upper: " << full << '\n';

    // Conversions
    int n = std::stoi("123");
    double pi = std::stod("3.14159");
    std::string back = std::to_string(n * 2);
    std::cout << std::format("n={} pi={:.2f} back='{}'\n", n, pi, back);

    // std::format: alignment and widths are great for tables
    std::cout << std::format("{:<10}|{:>8}|\n", "item", "price");
    std::cout << std::format("{:<10}|{:>8.2f}|\n", "coffee", 3.5);
    std::cout << std::format("{:<10}|{:>8.2f}|\n", "bagel", 2.25);

    // Robust input: check the stream state after every read.
    std::cout << "\nEnter a number then a line of text:\n";
    int value{};
    if (std::cin >> value) {
        std::cin.ignore(); // discard the space/newline after the number
        std::string rest;
        std::getline(std::cin, rest);
        std::cout << std::format("number={} rest='{}'\n", value, rest);
    } else {
        std::cout << "(no valid number on stdin)\n";
    }
}
