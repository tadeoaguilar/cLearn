// echo "4 8 15 16 23 42" | ./ex03_stats
#include <format>
#include <iostream>
#include <limits>

int main() {
    long long sum = 0;
    int count = 0;
    int min = std::numeric_limits<int>::max();
    int max = std::numeric_limits<int>::min();

    int x{};
    while (std::cin >> x) { // the loop stops at EOF or on non-numeric input
        ++count;
        sum += x;
        if (x < min) min = x;
        if (x > max) max = x;
    }

    if (count == 0) {
        std::cout << "No numbers given. Try: echo \"1 2 3\" | ./ex03_stats\n";
        return 0;
    }
    double avg = static_cast<double>(sum) / count;
    std::cout << std::format("count={} min={} max={} sum={} avg={:.2f}\n", count, min, max, sum, avg);
}
