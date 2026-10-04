// Public API of the textstats library. Only declarations live here;
// the definitions are in src/textstats.cpp.
#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ts {

struct Summary {
    std::size_t lines = 0;
    std::size_t words = 0;
    std::size_t chars = 0;
    double avg_word_length = 0.0;
};

// Splits on anything that is not a letter, digit or apostrophe; lowercases words.
std::vector<std::string> tokenize(std::string_view text);

std::size_t word_count(std::string_view text);

Summary summarize(std::string_view text);

std::map<std::string, std::size_t> word_frequencies(std::string_view text);

// The n most frequent words, ties broken alphabetically.
// Throws std::invalid_argument if n == 0.
std::vector<std::pair<std::string, std::size_t>> top_words(std::string_view text, std::size_t n);

} // namespace ts
