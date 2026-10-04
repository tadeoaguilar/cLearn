#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <vector>

long long sum_range(const std::vector<int>& v, std::size_t begin, std::size_t end) {
    return std::accumulate(v.begin() + static_cast<std::ptrdiff_t>(begin), v.begin() + static_cast<std::ptrdiff_t>(end), 0LL);
}

int main() {
    std::vector<int> data(10'000'000, 1);

    // Split the work into 4 async tasks
    const std::size_t parts = 4, chunk = data.size() / parts;
    std::vector<std::future<long long>> futures;
    for (std::size_t i = 0; i < parts; ++i) {
        std::size_t b = i * chunk, e = (i == parts - 1) ? data.size() : b + chunk;
        futures.push_back(std::async(std::launch::async, sum_range, std::cref(data), b, e));
    }
    long long total = 0;
    for (auto& f : futures) total += f.get(); // get() waits for each result
    std::cout << "parallel sum = " << total << '\n';

    // Exceptions travel through futures
    auto failing = std::async(std::launch::async, []() -> int { throw std::runtime_error("task failed"); });
    try {
        failing.get();
    } catch (const std::exception& e) {
        std::cout << "caught from other thread: " << e.what() << '\n';
    }

    // promise/future: hand a value from one thread to another manually
    std::promise<std::string> promise;
    std::future<std::string> result = promise.get_future();
    std::jthread producer([p = std::move(promise)]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        p.set_value("data from producer");
    });

    // wait_for lets you poll with a timeout
    while (result.wait_for(std::chrono::milliseconds(5)) != std::future_status::ready) std::cout << "waiting...\n";
    std::cout << result.get() << '\n';
}
