#include <compare>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>

class Time {
public:
    static constexpr int kSecondsPerDay = 24 * 60 * 60;

    Time(int h = 0, int m = 0, int s = 0) {
        if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) throw std::out_of_range("invalid time");
        secs_ = h * 3600 + m * 60 + s;
    }

    int hours() const { return secs_ / 3600; }
    int minutes() const { return secs_ / 60 % 60; }
    int seconds() const { return secs_ % 60; }

    Time& operator+=(int delta) {
        // % can be negative in C++, so add a full day before the final modulo
        secs_ = ((secs_ + delta) % kSecondsPerDay + kSecondsPerDay) % kSecondsPerDay;
        return *this;
    }
    friend Time operator+(Time t, int delta) { return t += delta; }

    std::string to_string() const { return std::format("{:02}:{:02}:{:02}", hours(), minutes(), seconds()); }

    auto operator<=>(const Time&) const = default;

private:
    int secs_{0}; // invariant: 0 <= secs_ < kSecondsPerDay
};

int main() {
    Time t{23, 59, 30};
    std::cout << t.to_string() << " + 45s = " << (t + 45).to_string() << '\n';
    std::cout << t.to_string() << " - 1h  = " << (t + -3600).to_string() << '\n';

    Time morning{8, 0, 0}, noon{12, 0, 0};
    std::cout << std::boolalpha << "morning < noon ? " << (morning < noon) << '\n';

    Time start{0, 0, 10};
    start += -20; // wraps backwards past midnight
    std::cout << "00:00:10 - 20s = " << start.to_string() << '\n';

    try {
        Time bad{25, 0, 0};
    } catch (const std::out_of_range& e) {
        std::cout << "error: " << e.what() << '\n';
    }
}
