// Usage: textstats_cli <file>        (or pipe text on stdin)
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include "textstats/textstats.hpp"

int main(int argc, char* argv[]) {
    std::string text;
    if (argc > 1) {
        std::ifstream in(argv[1]);
        if (!in) {
            std::cerr << "cannot open " << argv[1] << '\n';
            return 1;
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        text = ss.str();
    } else {
        text.assign(std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>());
    }

    auto s = ts::summarize(text);
    std::cout << std::format("lines: {}\nwords: {}\nchars: {}\navg word length: {:.2f}\n", s.lines, s.words, s.chars,
                             s.avg_word_length);
    if (s.words > 0) {
        std::cout << "top words:\n";
        for (const auto& [word, count] : ts::top_words(text, 5)) std::cout << std::format("  {:<12} {}\n", word, count);
    }
}
