#include <algorithm>
#include <format>
#include <iostream>
#include <map>
#include <numeric>
#include <ranges>
#include <string>
#include <vector>

struct Order {
    std::string customer;
    double amount;
    bool paid;
    int day;
};

int main() {
    const int today = 30;
    const std::vector<Order> orders{
        {"acme", 120.0, true, 2},  {"globex", 75.5, true, 25},  {"acme", 300.0, false, 26},
        {"initech", 42.0, true, 27}, {"globex", 210.0, true, 29}, {"umbrella", 999.0, true, 28},
        {"initech", 15.0, true, 10}, {"acme", 60.0, true, 30},
    };

    auto paid = orders | std::views::filter(&Order::paid);

    // 1) total revenue
    auto amounts = paid | std::views::transform(&Order::amount);
    double total = std::accumulate(amounts.begin(), amounts.end(), 0.0);
    std::cout << std::format("Total paid revenue: {:.2f}\n", total);

    // 2) recent paid orders sorted by amount desc
    auto recent = paid | std::views::filter([&](const Order& o) { return o.day >= today - 7; })
                       | std::ranges::to<std::vector>();
    std::ranges::sort(recent, std::ranges::greater{}, &Order::amount);
    std::cout << "Paid in the last 7 days:\n";
    for (const auto& o : recent) std::cout << std::format("  day {:>2}  {:<9} {:>8.2f}\n", o.day, o.customer, o.amount);

    // 3) revenue per customer + top 3
    std::map<std::string, double> per_customer;
    std::ranges::for_each(paid, [&](const Order& o) { per_customer[o.customer] += o.amount; });

    std::vector<std::pair<std::string, double>> ranking(per_customer.begin(), per_customer.end());
    std::ranges::sort(ranking, std::ranges::greater{}, &std::pair<std::string, double>::second);
    std::cout << "Top customers:\n";
    for (const auto& [name, revenue] : ranking | std::views::take(3)) std::cout << std::format("  {:<9} {:>8.2f}\n", name, revenue);
}
