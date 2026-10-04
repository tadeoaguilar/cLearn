// TDD: the tests at the bottom were written first; slugify() was written to make them pass.
#include <cctype>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// ---- the tiny framework from examples/02 (trimmed) ----
namespace mini {
struct TestCase { std::string name; std::function<void()> fn; };
inline std::vector<TestCase>& registry() { static std::vector<TestCase> t; return t; }
inline int g_failures = 0;
struct Registrar { Registrar(std::string n, std::function<void()> f) { registry().push_back({std::move(n), std::move(f)}); } };
} // namespace mini
#define MINI_CAT2(a, b) a##b
#define MINI_CAT(a, b) MINI_CAT2(a, b)
#define TEST(name) static void MINI_CAT(t_, __LINE__)(); static mini::Registrar MINI_CAT(r_, __LINE__){name, MINI_CAT(t_, __LINE__)}; static void MINI_CAT(t_, __LINE__)()
#define CHECK_EQ(a, b) do { auto _a = (a); auto _b = (b); if (!(_a == _b)) { ++mini::g_failures; std::cout << "    line " << __LINE__ << ": got \"" << _a << "\" expected \"" << _b << "\"\n"; } } while (false)

// ---- implementation ----
std::string slugify(std::string_view title) {
    std::string out;
    bool pending_dash = false;
    for (char ch : title) {
        auto c = static_cast<unsigned char>(ch);
        if (c < 128 && std::isalnum(c)) {
            if (pending_dash && !out.empty()) out += '-';
            pending_dash = false;
            out += static_cast<char>(std::tolower(c));
        } else if (c < 128) {
            pending_dash = true; // any ASCII separator/punctuation becomes (at most) one dash
        }
        // non-ASCII bytes are dropped
    }
    return out;
}

// ---- tests (written first) ----
TEST("basic title") { CHECK_EQ(slugify("Hello, World!"), std::string{"hello-world"}); }
TEST("collapses whitespace and symbols") { CHECK_EQ(slugify("  C++ is   FUN  "), std::string{"c-is-fun"}); }
TEST("empty input") { CHECK_EQ(slugify(""), std::string{""}); }
TEST("non-ascii dropped") { CHECK_EQ(slugify("Ünïcödé?"), std::string{"ncd"}); }
TEST("repeated dashes collapse") { CHECK_EQ(slugify("a--b"), std::string{"a-b"}); }
TEST("digits kept") { CHECK_EQ(slugify("Top 10 Tips"), std::string{"top-10-tips"}); }

int main() {
    int failed = 0;
    for (const auto& t : mini::registry()) {
        int before = mini::g_failures;
        t.fn();
        bool ok = before == mini::g_failures;
        failed += !ok;
        std::cout << (ok ? "[ PASS ] " : "[ FAIL ] ") << t.name << '\n';
    }
    return failed;
}
