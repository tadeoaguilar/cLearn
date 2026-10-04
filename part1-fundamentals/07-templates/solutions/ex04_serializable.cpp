#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
concept Serializable = requires(const T& t) {
    { t.serialize() } -> std::same_as<std::string>;
};

template <typename T>
std::string to_text(const T& value) {
    if constexpr (Serializable<T>) {
        return value.serialize();
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else {
        static_assert(sizeof(T) == 0, "to_text: unsupported type"); // fires only if instantiated
    }
}

template <typename T>
std::string save_all(const std::vector<T>& items) {
    std::string out;
    for (const auto& item : items) out += to_text(item) + '\n';
    return out;
}

struct User {
    int id;
    std::string name;
    std::string serialize() const { return std::to_string(id) + ";" + name; }
};

int main() {
    std::cout << save_all(std::vector<User>{{1, "ada"}, {2, "grace"}});
    std::cout << save_all(std::vector<int>{10, 20, 30});
    std::cout << save_all(std::vector<std::string>{"plain", "strings"});
    static_assert(Serializable<User> && !Serializable<int>);
}
