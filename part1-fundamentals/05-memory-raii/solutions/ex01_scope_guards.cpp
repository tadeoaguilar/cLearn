#include <chrono>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class ScopeTimer {
public:
    explicit ScopeTimer(std::string label) : label_{std::move(label)}, start_{std::chrono::steady_clock::now()} {}
    ~ScopeTimer() {
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_);
        std::cout << "[timer] " << label_ << ": " << us.count() << " us\n";
    }
    ScopeTimer(const ScopeTimer&) = delete;
    ScopeTimer& operator=(const ScopeTimer&) = delete;

private:
    std::string label_;
    std::chrono::steady_clock::time_point start_;
};

class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> on_exit) : fn_{std::move(on_exit)} {}
    ~ScopeGuard() {
        if (active_) fn_();
    }
    void dismiss() { active_ = false; }
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> fn_;
    bool active_ = true;
};

// Adds an item and "commits" only if validation passes; otherwise the guard rolls back.
void add_item(std::vector<std::string>& items, const std::string& item) {
    items.push_back(item);
    ScopeGuard rollback{[&items] {
        items.pop_back();
        std::cout << "  rolled back\n";
    }};
    if (item.empty()) throw std::invalid_argument("empty item");
    rollback.dismiss(); // success: keep the change
}

int main() {
    {
        ScopeTimer t{"sum loop"};
        volatile long long sum = 0; // volatile so the optimizer can't delete the loop
        for (int i = 0; i < 1'000'000; ++i) sum = sum + i;
    }

    std::vector<std::string> items;
    add_item(items, "apple");
    try {
        add_item(items, "");
    } catch (const std::exception& e) {
        std::cout << "  error: " << e.what() << '\n';
    }
    std::cout << "items.size() = " << items.size() << " (only 'apple')\n";
}
