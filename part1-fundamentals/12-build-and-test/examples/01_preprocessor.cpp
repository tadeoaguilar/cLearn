// The preprocessor runs BEFORE the compiler and only manipulates text.
// See its output with: clang++ -E 01_preprocessor.cpp | tail -40
#include <iostream>

#define SQUARE_BAD(x) x * x          // no parentheses: precedence bug
#define SQUARE_OK(x) ((x) * (x))     // still evaluates x twice!
constexpr int square(int x) { return x * x; } // the right tool: typed, scoped, evaluated once

#define STRINGIFY(x) #x              // # turns a token into a string literal
#define CONCAT(a, b) a##b            // ## pastes tokens together

#ifndef APP_VERSION                  // can be set from the command line: -DAPP_VERSION=\"2.0\"
#define APP_VERSION "1.0-dev"
#endif

int main() {
    std::cout << "SQUARE_BAD(2 + 1) = " << SQUARE_BAD(2 + 1) << "  (expands to 2 + 1 * 2 + 1 = 5!)\n";
    std::cout << "SQUARE_OK(2 + 1)  = " << SQUARE_OK(2 + 1) << '\n';

    // A macro pastes its argument text twice, so side effects happen twice.
    // (SQUARE_OK(i++) would even be undefined behavior: two unsequenced writes to i.)
    int calls = 0;
    auto next = [&calls] { return ++calls; };
    int r = SQUARE_OK(next());
    std::cout << "SQUARE_OK(next()) = " << r << ", next() ran " << calls << " times\n";
    calls = 0;
    r = square(next());
    std::cout << "square(next())    = " << r << ", next() ran " << calls << " time\n";

    int CONCAT(my, Var) = 7; // declares myVar
    std::cout << STRINGIFY(myVar) << " = " << myVar << '\n';

    std::cout << "version: " << APP_VERSION << '\n';
    std::cout << "compiled " << __DATE__ << " in " << __FILE__ << ':' << __LINE__ << '\n';

#if defined(_WIN32)
    std::cout << "platform: Windows\n";
#elif defined(__APPLE__)
    std::cout << "platform: macOS\n";
#elif defined(__linux__)
    std::cout << "platform: Linux\n";
#endif

#ifdef NDEBUG
    std::cout << "release build (asserts disabled)\n";
#else
    std::cout << "debug build (asserts enabled)\n";
#endif
}
