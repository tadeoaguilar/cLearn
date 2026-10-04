#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// A reusable thread-safe queue. close() wakes all waiters so they can exit.
template <typename T>
class BlockingQueue {
public:
    void push(T value) {
        {
            std::lock_guard lock(m_);
            q_.push(std::move(value));
        }
        cv_.notify_one(); // notify after unlocking: the woken thread can grab the lock immediately
    }

    // Blocks until an item is available or the queue is closed and empty.
    std::optional<T> pop() {
        std::unique_lock lock(m_);
        cv_.wait(lock, [&] { return !q_.empty() || closed_; }); // predicate handles spurious wakeups
        if (q_.empty()) return std::nullopt;
        T v = std::move(q_.front());
        q_.pop();
        return v;
    }

    void close() {
        {
            std::lock_guard lock(m_);
            closed_ = true;
        }
        cv_.notify_all();
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::queue<T> q_;
    bool closed_ = false;
};

int main() {
    BlockingQueue<std::string> jobs;
    std::mutex out_m; // protects std::cout so lines don't interleave
    std::vector<int> processed(3, 0);

    {
        std::vector<std::jthread> consumers;
        for (int id = 0; id < 3; ++id) {
            consumers.emplace_back([&, id] {
                while (auto job = jobs.pop()) {
                    ++processed[static_cast<std::size_t>(id)]; // each thread writes only its own slot: no race
                    std::lock_guard lock(out_m);
                    std::cout << "consumer " << id << " handled " << *job << '\n';
                }
            });
        }

        for (int i = 1; i <= 9; ++i) jobs.push("job-" + std::to_string(i));
        jobs.close(); // consumers drain the queue, then pop() returns nullopt
    } // consumers joined

    for (std::size_t i = 0; i < processed.size(); ++i) std::cout << "consumer " << i << ": " << processed[i] << " jobs\n";
}
