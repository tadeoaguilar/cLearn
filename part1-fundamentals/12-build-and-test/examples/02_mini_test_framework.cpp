// How test frameworks like doctest/Catch2/GoogleTest work, in ~60 lines:
//  1. a macro defines a function AND registers it in a global list before main runs
//  2. CHECK records failures with file/line instead of aborting
//  3. main runs every registered test and reports.
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace mini {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests; // function-local static: initialized on first use
    return tests;
}

inline int g_failures = 0;

struct Registrar {
    Registrar(std::string name, std::function<void()> fn) { registry().push_back({std::move(name), std::move(fn)}); }
};

inline int run_all() {
    int failed_tests = 0;
    for (const auto& t : registry()) {
        int before = g_failures;
        t.fn();
        bool ok = g_failures == before;
        failed_tests += !ok;
        std::cout << (ok ? "[ PASS ] " : "[ FAIL ] ") << t.name << '\n';
    }
    std::cout << registry().size() - static_cast<std::size_t>(failed_tests) << '/' << registry().size() << " tests passed\n";
    return failed_tests == 0 ? 0 : 1;
}

} // namespace mini

// Unique names per test via __LINE__ (two-step concat so the macro expands)
#define MINI_CAT2(a, b) a##b
#define MINI_CAT(a, b) MINI_CAT2(a, b)
#define TEST(name)                                                                    \
    static void MINI_CAT(test_fn_, __LINE__)();                                       \
    static mini::Registrar MINI_CAT(test_reg_, __LINE__){name, MINI_CAT(test_fn_, __LINE__)}; \
    static void MINI_CAT(test_fn_, __LINE__)()

#define CHECK(expr)                                                                         \
    do {                                                                                    \
        if (!(expr)) {                                                                      \
            ++mini::g_failures;                                                             \
            std::cout << "    " << __FILE__ << ':' << __LINE__ << ": CHECK(" #expr ") failed\n"; \
        }                                                                                   \
    } while (false)

// ---------------- code under test ----------------
int add(int a, int b) { return a + b; }
std::string shout(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s + "!";
}

// ---------------- tests ----------------
TEST("add works for small numbers") {
    CHECK(add(2, 2) == 4);
    CHECK(add(-1, 1) == 0);
}

TEST("shout uppercases and adds !") {
    CHECK(shout("hi") == "HI!");
    CHECK(shout("") == "!");
}

TEST("a deliberately failing test (to see the report)") { CHECK(add(2, 2) == 5); }

int main() {
    mini::run_all();
    return 0; // always 0 here so check.sh --run stays quiet; real runners return the failure status
}
