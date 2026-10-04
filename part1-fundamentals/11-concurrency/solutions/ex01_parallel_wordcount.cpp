#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using Counts = std::unordered_map<std::string, int>;

Counts count_range(const std::vector<std::string>& words, std::size_t b, std::size_t e) {
    Counts c;
    for (std::size_t i = b; i < e; ++i) ++c[words[i]];
    return c;
}

int main() {
    const std::vector<std::string> vocab{"alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta", "iota", "kappa",
                                         "lambda", "mu", "nu", "xi", "omicron", "pi", "rho", "sigma", "tau", "upsilon"};
    std::mt19937 rng{7};
    std::uniform_int_distribution<std::size_t> pick(0, vocab.size() - 1);
    std::vector<std::string> words(2'000'000);
    for (auto& w : words) w = vocab[pick(rng)];

    using clock = std::chrono::steady_clock;

    auto t0 = clock::now();
    Counts serial = count_range(words, 0, words.size());
    auto serial_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();

    t0 = clock::now();
    const std::size_t n = std::max(2u, std::thread::hardware_concurrency());
    std::vector<Counts> partial(n);
    {
        std::vector<std::jthread> threads;
        const std::size_t chunk = words.size() / n;
        for (std::size_t i = 0; i < n; ++i) {
            std::size_t b = i * chunk, e = (i + 1 == n) ? words.size() : b + chunk;
            threads.emplace_back([&, i, b, e] { partial[i] = count_range(words, b, e); }); // own slot: no race
        }
    }
    Counts merged;
    for (const auto& p : partial)
        for (const auto& [w, c] : p) merged[w] += c;
    auto parallel_ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();

    std::cout << "serial:   " << serial_ms << " ms\n";
    std::cout << "parallel: " << parallel_ms << " ms with " << n << " threads\n";
    std::cout << "results match: " << std::boolalpha << (serial == merged) << '\n';
    std::cout << "'alpha' appears " << merged["alpha"] << " times\n";
}
