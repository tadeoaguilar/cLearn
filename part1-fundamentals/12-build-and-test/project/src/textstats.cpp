#include "textstats/textstats.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace ts {

namespace { // internal helpers: invisible outside this file

bool is_word_char(char c) {
    auto u = static_cast<unsigned char>(c);
    return std::isalnum(u) || c == '\'';
}

} // namespace

std::vector<std::string> tokenize(std::string_view text) {
    std::vector<std::string> words;
    std::string current;
    for (char c : text) {
        if (is_word_char(c)) {
            current += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (!current.empty()) {
            words.push_back(std::move(current));
            current.clear();
        }
    }
    if (!current.empty()) words.push_back(std::move(current));
    return words;
}

std::size_t word_count(std::string_view text) { return tokenize(text).size(); }

Summary summarize(std::string_view text) {
    Summary s;
    s.chars = text.size();
    if (!text.empty()) s.lines = static_cast<std::size_t>(std::ranges::count(text, '\n')) + (text.back() == '\n' ? 0 : 1);
    auto words = tokenize(text);
    s.words = words.size();
    std::size_t letters = 0;
    for (const auto& w : words) letters += w.size();
    s.avg_word_length = words.empty() ? 0.0 : static_cast<double>(letters) / static_cast<double>(words.size());
    return s;
}

std::map<std::string, std::size_t> word_frequencies(std::string_view text) {
    std::map<std::string, std::size_t> freq;
    for (auto& w : tokenize(text)) ++freq[std::move(w)];
    return freq;
}

std::vector<std::pair<std::string, std::size_t>> top_words(std::string_view text, std::size_t n) {
    if (n == 0) throw std::invalid_argument("top_words: n must be > 0");
    auto freq = word_frequencies(text);
    std::vector<std::pair<std::string, std::size_t>> v(freq.begin(), freq.end());
    std::ranges::stable_sort(v, std::ranges::greater{}, &std::pair<std::string, std::size_t>::second);
    if (v.size() > n) v.resize(n);
    return v; // map iteration was alphabetical and stable_sort keeps that order for ties
}

} // namespace ts
