#include <chrono>
#include <format>
#include <iostream>
#include <print>
#include <thread>

struct Vec3 {
    double x, y, z;
};

// Teach std::format about Vec3
template <>
struct std::formatter<Vec3> : std::formatter<double> {
    auto format(const Vec3& v, std::format_context& ctx) const {
        auto out = ctx.out();
        out = std::format_to(out, "(");
        out = std::formatter<double>::format(v.x, ctx); // reuse the double format spec, e.g. {:.2f}
        out = std::format_to(out, ", ");
        out = std::formatter<double>::format(v.y, ctx);
        out = std::format_to(out, ", ");
        out = std::formatter<double>::format(v.z, ctx);
        return std::format_to(out, ")");
    }
};

int main() {
    using namespace std::chrono_literals;

    std::cout << std::format("[{:>8}] [{:<8}] [{:^8}]\n", "right", "left", "center");
    std::cout << std::format("{:.3f} {:+d} {:#x} {:#b} {:08.2f} {:e}\n", 3.14159, 42, 255, 5, 3.5, 12345.678);
    std::cout << std::format("{0} {1} {0}\n", "a", "b"); // positional
    std::cout << std::format("{:*^20}\n", " menu ");     // fill character

    Vec3 pos{1.23456, -2.5, 10};
    std::println("player at {:.2f}", pos); // C++23 std::println uses std::format underneath

    // Durations are typed with units
    auto total = 1500ms + 2s;
    std::println("total = {} = {}", total, std::chrono::duration_cast<std::chrono::seconds>(total));

    // Measuring time with steady_clock
    auto start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(20ms);
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    std::println("slept ~{}", elapsed);

    // Calendar dates (C++20)
    using namespace std::chrono;
    year_month_day launch{2026y / October / 3d};
    sys_days d = launch;
    year_month_day later{d + days{100}};
    std::println("{} + 100 days = {}", launch, later);
    std::println("weekday of launch: {}", weekday{d});
}
