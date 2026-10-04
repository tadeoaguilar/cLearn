// Build with -fsanitize=thread to see ThreadSanitizer report the race in `racy`.
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

constexpr int kThreads = 4;
constexpr int kIncrements = 200'000;

template <typename F>
void run(const char* label, F body) {
    auto start = std::chrono::steady_clock::now();
    {
        std::vector<std::jthread> ts;
        for (int i = 0; i < kThreads; ++i) ts.emplace_back(body);
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    std::cout << label << " (" << ms << " ms): ";
}

int main() {
    // 1) DATA RACE: undefined behavior. Usually prints less than expected.
    int racy = 0;
    run("racy  ", [&] {
        for (int i = 0; i < kIncrements; ++i) ++racy;
    });
    std::cout << racy << " (expected " << kThreads * kIncrements << ")\n";

    // 2) Mutex: correct, but every increment takes a lock
    int guarded = 0;
    std::mutex m;
    run("mutex ", [&] {
        for (int i = 0; i < kIncrements; ++i) {
            std::lock_guard lock(m);
            ++guarded;
        }
    });
    std::cout << guarded << '\n';

    // 3) Atomic: correct and lock-free
    std::atomic<int> atomic_count{0};
    run("atomic", [&] {
        for (int i = 0; i < kIncrements; ++i) atomic_count.fetch_add(1, std::memory_order_relaxed);
    });
    std::cout << atomic_count.load() << '\n';

    // 4) Best: share nothing, combine at the end
    std::atomic<int> total{0};
    run("local ", [&] {
        int local = 0;
        for (int i = 0; i < kIncrements; ++i) ++local;
        total += local; // one synchronized operation per thread
    });
    std::cout << total.load() << '\n';
}
