// The smallest useful C++ program.
// Build: clang++ -std=c++23 01_hello_world.cpp -o hello && ./hello
#include <iostream>

int main() {
    std::cout << "Hello, C++!\n";
    std::cout << "2 + 3 = " << 2 + 3 << '\n'; // << chains multiple values
    return 0;                                  // 0 means "success" to the shell (echo $?)
}
