// A small, real thread pool: N workers pulling std::function<void()> jobs from a queue.
// submit() returns a std::future for the job's result.
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t n = std::thread::hardware_concurrency()) {
        for (std::size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> job;
                    {
                        std::unique_lock lock(m_);
                        cv_.wait(lock, [this] { return stopping_ || !jobs_.empty(); });
                        if (stopping_ && jobs_.empty()) return;
                        job = std::move(jobs_.front());
                        jobs_.pop();
                    }
                    job(); // run outside the lock
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard lock(m_);
            stopping_ = true;
        }
        cv_.notify_all();
        // jthreads join automatically after this body ends
    }

    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using R = std::invoke_result_t<F, Args...>;
        // packaged_task is move-only, std::function needs copyable → wrap in shared_ptr
        auto task = std::make_shared<std::packaged_task<R()>>(
            [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable { return f(args...); });
        auto fut = task->get_future();
        {
            std::lock_guard lock(m_);
            jobs_.emplace([task] { (*task)(); });
        }
        cv_.notify_one();
        return fut;
    }

private:
    std::mutex m_;
    std::condition_variable cv_;
    std::queue<std::function<void()>> jobs_;
    bool stopping_ = false;
    std::vector<std::jthread> workers_; // declared last → destroyed (joined) first
};

bool is_prime(long long n) {
    if (n < 2) return false;
    for (long long d = 2; d * d <= n; ++d)
        if (n % d == 0) return false;
    return true;
}

int main() {
    ThreadPool pool{4};
    std::vector<std::future<int>> results;
    const long long block = 250'000;
    for (int i = 0; i < 8; ++i) {
        results.push_back(pool.submit([](long long from, long long to) {
            int count = 0;
            for (long long n = from; n < to; ++n) count += is_prime(n);
            return count;
        }, i * block, (i + 1) * block));
    }
    int total = 0;
    for (auto& f : results) total += f.get();
    std::cout << "primes below " << 8 * block << ": " << total << '\n';

    auto hello = pool.submit([] { return std::string{"pool says hi"}; });
    std::cout << hello.get() << '\n';
}
