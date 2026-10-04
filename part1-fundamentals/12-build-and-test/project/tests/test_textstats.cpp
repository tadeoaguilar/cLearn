#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <stdexcept>

#include "textstats/textstats.hpp"

TEST_CASE("tokenize splits on non-word characters and lowercases") {
    auto words = ts::tokenize("Hello, World! It's C++20.");
    REQUIRE(words.size() == 5);
    CHECK(words[0] == "hello");
    CHECK(words[1] == "world");
    CHECK(words[2] == "it's");
    CHECK(words[3] == "c");
    CHECK(words[4] == "20");
}

TEST_CASE("word_count") {
    CHECK(ts::word_count("") == 0);
    CHECK(ts::word_count("   ") == 0);
    CHECK(ts::word_count("one") == 1);
    CHECK(ts::word_count("one two  three\nfour") == 4);

    SUBCASE("punctuation alone is not a word") { CHECK(ts::word_count(" , . ! ") == 0); }
}

TEST_CASE("summarize counts lines, words and characters") {
    auto s = ts::summarize("ab cd\nefg\n");
    CHECK(s.lines == 2);
    CHECK(s.words == 3);
    CHECK(s.chars == 10);
    CHECK(s.avg_word_length == doctest::Approx(7.0 / 3.0));

    SUBCASE("last line without newline still counts") { CHECK(ts::summarize("a\nb").lines == 2); }
    SUBCASE("empty text") {
        auto e = ts::summarize("");
        CHECK(e.lines == 0);
        CHECK(e.avg_word_length == 0.0);
    }
}

TEST_CASE("top_words orders by frequency, then alphabetically") {
    auto top = ts::top_words("b a c b a b", 2);
    REQUIRE(top.size() == 2);
    CHECK(top[0] == std::pair<std::string, std::size_t>{"b", 3});
    CHECK(top[1] == std::pair<std::string, std::size_t>{"a", 2});

    SUBCASE("n larger than vocabulary returns everything") { CHECK(ts::top_words("x y", 10).size() == 2); }
    SUBCASE("n == 0 is an error") { CHECK_THROWS_AS(ts::top_words("x", 0), std::invalid_argument); }
}
