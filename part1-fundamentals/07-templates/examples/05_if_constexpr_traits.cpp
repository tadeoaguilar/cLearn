#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// if constexpr: branches that don't apply to T are discarded at compile time
template <typename T>
std::string describe(const T& value) {
    if constexpr (std::is_same_v<T, bool>) {
        return value ? "bool:true" : "bool:false";
    } else if constexpr (std::is_integral_v<T>) {
        return "integer:" + std::to_string(value);
    } else if constexpr (std::is_floating_point_v<T>) {
        return "float:" + std::to_string(value);
    } else if constexpr (std::is_convertible_v<T, std::string>) {
        return "string:" + std::string(value);
    } else if constexpr (std::is_pointer_v<T>) {
        return value ? "pointer to " + describe(*value) : "null pointer";
    } else {
        return "unknown type of size " + std::to_string(sizeof(T));
    }
}

// Classic full specialization
template <typename T>
struct TypeName { static constexpr const char* value = "?"; };
template <> struct TypeName<int> { static constexpr const char* value = "int"; };
template <> struct TypeName<double> { static constexpr const char* value = "double"; };

// Partial specialization: any vector
template <typename T>
struct TypeName<std::vector<T>> {
    static std::string value() { return std::string("vector<") + TypeName<T>::value + ">"; }
};

// Type transformations
template <typename T>
void inspect() {
    using Clean = std::remove_cvref_t<T>; // strip const/volatile/reference
    std::cout << std::boolalpha << "  is_reference=" << std::is_reference_v<T>
              << " is_const(after strip ref)=" << std::is_const_v<std::remove_reference_t<T>>
              << " clean is int? " << std::is_same_v<Clean, int> << '\n';
}

int main() {
    int x = 5;
    std::cout << describe(true) << '\n'
              << describe(42) << '\n'
              << describe(3.5) << '\n'
              << describe("hello") << '\n'
              << describe(&x) << '\n'
              << describe(std::vector<int>{}) << '\n';

    std::cout << TypeName<int>::value << ' ' << TypeName<double>::value << ' ' << TypeName<char>::value << '\n';
    std::cout << TypeName<std::vector<int>>::value() << '\n';

    std::cout << "inspect<const int&>:\n";
    inspect<const int&>();

    using Choice = std::conditional_t<(sizeof(void*) == 8), long long, int>;
    std::cout << "pointer-sized int has " << sizeof(Choice) << " bytes\n";
}
