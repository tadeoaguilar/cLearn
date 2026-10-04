#include <atomic>
#include <chrono>
#include <iostream>
#include <stop_token>
#include <thread>

int main() {
    using namespace std::chrono_literals;
    std::atomic<int> ticks{0};

    std::jthread game_loop([&ticks](std::stop_token st) {
        while (!st.stop_requested()) { // cooperative: the thread checks the token
            ++ticks;
            std::this_thread::sleep_for(10ms); // "simulate a frame"
        }
        std::cout << "game loop: stop requested, cleaning up\n";
    });

    // A callback that runs when stop is requested
    std::stop_callback on_stop(game_loop.get_stop_token(), [] { std::cout << "stop callback fired\n"; });

    std::this_thread::sleep_for(100ms);
    game_loop.request_stop(); // the destructor would also do this, then join
    game_loop.join();
    std::cout << "ran ~" << ticks.load() << " frames\n";
}
