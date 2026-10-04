#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

void greet(const std::string& name, int times) {
    for (int i = 0; i < times; ++i) {
        // Output from several threads can interleave; build the line first
        std::string line = "hello from " + name + " #" + std::to_string(i) + "\n";
        std::cout << line;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void fill(std::vector<int>& out, int value) { out.assign(5, value); }

int main() {
    std::cout << "hardware threads: " << std::thread::hardware_concurrency() << '\n';

    std::thread t1(greet, "t1", 3); // args are copied into the thread
    std::thread t2(greet, "t2", 3);
    t1.join(); // wait; forgetting join() → std::terminate when t1 is destroyed
    t2.join();

    // Passing a reference requires std::ref
    std::vector<int> data;
    std::thread t3(fill, std::ref(data), 7);
    t3.join();
    std::cout << "data[0] = " << data[0] << '\n';

    // jthread joins automatically (RAII)
    {
        std::vector<std::jthread> workers;
        for (int i = 0; i < 3; ++i)
            workers.emplace_back([i] { std::cout << ("worker " + std::to_string(i) + " id done\n"); });
    } // all joined here

    std::cout << "main thread id: " << std::this_thread::get_id() << '\n';
}
