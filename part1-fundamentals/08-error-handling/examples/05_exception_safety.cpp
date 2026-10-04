#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// --- Strong guarantee via "do the work on a copy, then commit with swap" ---
class Roster {
public:
    void add_all(const std::vector<std::string>& names) {
        std::vector<std::string> copy = names_; // work on a copy
        for (const auto& n : names) {
            if (n.empty()) throw std::invalid_argument("empty name");
            copy.push_back(n);
        }
        names_.swap(copy); // noexcept commit: all or nothing
    }

    // Only the BASIC guarantee: partial changes remain after a throw
    void add_all_basic(const std::vector<std::string>& names) {
        for (const auto& n : names) {
            if (n.empty()) throw std::invalid_argument("empty name");
            names_.push_back(n);
        }
    }

    std::size_t size() const { return names_.size(); }

private:
    std::vector<std::string> names_;
};

// --- Why move constructors should be noexcept ---
struct Tracked {
    std::string label;
    explicit Tracked(std::string l) : label{std::move(l)} {}
    Tracked(const Tracked& o) : label{o.label} { std::cout << "copy "; }
    Tracked(Tracked&& o) noexcept : label{std::move(o.label)} { std::cout << "move "; } // try removing noexcept!
    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&&) noexcept = default;
};

int main() {
    Roster r;
    r.add_all({"Ada", "Grace"});
    try {
        r.add_all({"Linus", "", "Bjarne"});
    } catch (const std::exception& e) {
        std::cout << "strong: failed (" << e.what() << "), size still " << r.size() << '\n';
    }
    try {
        r.add_all_basic({"Linus", "", "Bjarne"});
    } catch (const std::exception& e) {
        std::cout << "basic:  failed (" << e.what() << "), size now " << r.size() << " (Linus got in!)\n";
    }

    std::cout << "vector growth with noexcept move: ";
    std::vector<Tracked> v;
    for (int i = 0; i < 5; ++i) v.emplace_back("t" + std::to_string(i));
    std::cout << "\n(remove noexcept from the move ctor and the reallocations will COPY instead)\n";
}
