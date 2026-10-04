#include <atomic>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

class SettingsCache {
public:
    std::optional<std::string> get(const std::string& key) const {
        std::shared_lock lock(m_); // many readers at once
        if (auto it = data_.find(key); it != data_.end()) return it->second; // copy out under the lock
        return std::nullopt;
    }
    void set(const std::string& key, std::string value) {
        std::unique_lock lock(m_); // exclusive
        data_[key] = std::move(value);
    }

private:
    mutable std::shared_mutex m_; // mutable: locking in a const method
    std::map<std::string, std::string> data_;
};

int main() {
    using namespace std::chrono_literals;
    SettingsCache cache;
    cache.set("theme", "dark-dark-dark-dark");

    std::atomic<bool> stop{false};
    std::atomic<long> reads{0}, writes{0}, torn{0};

    {
        std::vector<std::jthread> threads;
        for (int r = 0; r < 6; ++r) {
            threads.emplace_back([&] {
                while (!stop) {
                    auto v = cache.get("theme");
                    // Invariant: the value is always one word repeated 4 times
                    if (v && v->substr(0, v->size() / 4 + 1) + v->substr(0, v->size() / 4 + 1) +
                                     v->substr(0, v->size() / 4 + 1) + v->substr(0, v->size() / 4) != *v)
                        ++torn;
                    ++reads;
                }
            });
        }
        threads.emplace_back([&] {
            const char* themes[] = {"dark", "light", "solarized"};
            for (int i = 0; !stop; ++i) {
                std::string t = themes[i % 3];
                cache.set("theme", t + "-" + t + "-" + t + "-" + t);
                ++writes;
                std::this_thread::sleep_for(1ms);
            }
        });
        std::this_thread::sleep_for(200ms);
        stop = true;
    }
    std::cout << "reads: " << reads << ", writes: " << writes << ", torn reads: " << torn << " (must be 0)\n";
}
