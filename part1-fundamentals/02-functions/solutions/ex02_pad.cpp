#include <iostream>
#include <string>

std::string pad(const std::string& s, std::size_t width, char fill = ' ', bool align_left = true) {
    if (s.size() >= width) return s;
    std::string padding(width - s.size(), fill); // string of N copies of fill
    return align_left ? s + padding : padding + s;
}

int main() {
    std::cout << '[' << pad("left", 10) << "]\n";
    std::cout << '[' << pad("right", 10, ' ', false) << "]\n";
    std::cout << '[' << pad("dots", 10, '.') << "]\n";
    std::cout << '[' << pad("too long already", 5) << "]\n\n";

    struct Row { std::string name; double price; };
    for (const Row& r : {Row{"Coffee", 3.5}, Row{"Croissant", 2.75}, Row{"Tea", 2.0}}) {
        std::string price = std::to_string(r.price);
        price = price.substr(0, price.find('.') + 3); // keep 2 decimals
        std::cout << pad(r.name, 12, '.') << pad(price, 8, ' ', false) << '\n';
    }
}
