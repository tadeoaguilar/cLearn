#include <cctype>
#include <iostream>
#include <string>

bool is_palindrome(const std::string& s) {
    // Two indexes walking towards each other, skipping non-alphanumerics.
    std::size_t i = 0;
    std::size_t j = s.empty() ? 0 : s.size() - 1;
    while (i < j) {
        auto a = static_cast<unsigned char>(s[i]);
        auto b = static_cast<unsigned char>(s[j]);
        if (!std::isalnum(a)) { ++i; continue; }
        if (!std::isalnum(b)) { --j; continue; }
        if (std::tolower(a) != std::tolower(b)) return false;
        ++i;
        --j;
    }
    return true;
}

int main() {
    for (const std::string s : {"A man, a plan, a canal: Panama", "racecar", "hello", "", "No 'x' in Nixon"}) {
        std::cout << '"' << s << "\" -> " << std::boolalpha << is_palindrome(s) << '\n';
    }
}
