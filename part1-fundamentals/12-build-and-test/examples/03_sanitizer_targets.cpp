// Bugs that sanitizers catch. Nothing bad runs unless you pass an argument.
//   clang++ -std=c++23 -g -fsanitize=address,undefined 03_sanitizer_targets.cpp -o san
//   ./san overflow | ./san uaf | ./san signed | ./san leak
#include <climits>
#include <iostream>
#include <string_view>
#include <vector>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "pass one of: overflow, uaf, signed, leak\n";
        return 0;
    }
    std::string_view which = argv[1];

    if (which == "overflow") {
        std::vector<int> v(4);
        int* p = v.data();
        p[4] = 1; // heap-buffer-overflow: ASan reports the exact line
    } else if (which == "uaf") {
        int* p = new int(5);
        delete p;
        std::cout << *p << '\n'; // heap-use-after-free
    } else if (which == "signed") {
        int big = INT_MAX;
        volatile int one = 1;
        std::cout << big + one << '\n'; // signed integer overflow: UBSan reports it
    } else if (which == "leak") {
        new int[100]; // LeakSanitizer (on Linux; on macOS use `leaks --atExit -- ./san leak`)
    }
    std::cout << "done (without sanitizers the bug may go unnoticed!)\n";
}
