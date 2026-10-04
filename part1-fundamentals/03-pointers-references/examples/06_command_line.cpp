// Run: ./program hello "two words" 42
#include <iostream>
#include <span>
#include <string_view>

int main(int argc, char* argv[]) {
    std::cout << "argc = " << argc << '\n';
    // Wrap argv in a span of string_views: modern and bounds-aware
    std::span<char*> args(argv, static_cast<std::size_t>(argc));
    for (std::size_t i = 0; i < args.size(); ++i) {
        std::string_view arg = args[i];
        std::cout << "argv[" << i << "] = \"" << arg << "\" (" << arg.size() << " chars)\n";
    }
}
