#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

template <typename T>
class BlockingQueue {
public:
    void push(T v) {
        { std::lock_guard l(m_); q_.push(std::move(v)); }
        cv_.notify_one();
    }
    std::optional<T> pop() {
        std::unique_lock l(m_);
        cv_.wait(l, [&] { return !q_.empty() || closed_; });
        if (q_.empty()) return std::nullopt;
        T v = std::move(q_.front());
        q_.pop();
        return v;
    }
    void close() {
        { std::lock_guard l(m_); closed_ = true; }
        cv_.notify_all();
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::queue<T> q_;
    bool closed_ = false;
};

int main() {
    BlockingQueue<int> numbers, squares;
    long long sum = 0;
    {
        std::jthread generator([&] {
            for (int i = 1; i <= 20; ++i) numbers.push(i);
            numbers.close();
        });
        std::jthread squarer([&] {
            while (auto n = numbers.pop()) squares.push(*n * *n);
            squares.close(); // propagate "end of stream"
        });
        std::jthread sink([&] {
            while (auto s = squares.pop()) sum += *s; // only this thread touches sum until join
        });
    } // all three joined here, so reading sum below is safe
    std::cout << "sum of squares 1..20 = " << sum << " (expected 2870)\n";
}
