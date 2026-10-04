// Overloading, default arguments and [[nodiscard]].
#include <iostream>
#include <numbers>
#include <string>

double area(double radius) { return std::numbers::pi * radius * radius; }
double area(double width, double height) { return width * height; }

void print(int x)                { std::cout << "int: " << x << '\n'; }
void print(double x)             { std::cout << "double: " << x << '\n'; }
void print(const std::string& s) { std::cout << "string: " << s << '\n'; }

std::string greet(const std::string& name, const std::string& greeting = "Hello",
                  char punctuation = '!') {
    return greeting + ", " + name + punctuation;
}

[[nodiscard]] int parse_port(const std::string& s) { return std::stoi(s); }

int main() {
    std::cout << "circle r=1:  " << area(1.0) << '\n';
    std::cout << "rect 2x3:    " << area(2.0, 3.0) << '\n';

    print(42);
    print(4.2);
    print(std::string{"hello"});
    print('A'); // char → promoted to int (exact match for int wins over double)

    std::cout << greet("Ada") << '\n';
    std::cout << greet("Ada", "Welcome") << '\n';
    std::cout << greet("Ada", "Goodbye", '.') << '\n';

    // parse_port("8080");  // warning: ignoring return value marked nodiscard
    int port = parse_port("8080");
    std::cout << "port: " << port << '\n';
}
