#include <iostream>
#include <string>
#include <utility>
#include <vector>

class Track {
public:
    explicit Track(std::string n) : name_{std::move(n)} { std::cout << "  ctor(" << name_ << ")\n"; }
    Track(const Track& o) : name_{o.name_} { std::cout << "  COPY(" << name_ << ")\n"; }
    Track(Track&& o) noexcept : name_{std::move(o.name_)} { std::cout << "  move(" << name_ << ")\n"; }
    Track& operator=(const Track& o) { name_ = o.name_; std::cout << "  COPY=(" << name_ << ")\n"; return *this; }
    Track& operator=(Track&& o) noexcept { name_ = std::move(o.name_); std::cout << "  move=(" << name_ << ")\n"; return *this; }
    ~Track() = default;
    const std::string& name() const { return name_; }

private:
    std::string name_;
};

Track make_track(const std::string& n) {
    Track t{n};
    return t; // NRVO: usually neither copy nor move
}

int main() {
    std::cout << "1) copy from lvalue:\n";
    Track a{"a"};
    Track b = a;

    std::cout << "2) move from std::move(lvalue):\n";
    Track c = std::move(a);

    std::cout << "3) return by value (elided):\n";
    Track d = make_track("d");

    std::cout << "4) assignment:\n";
    b = c;               // copy-assign
    b = make_track("e"); // move-assign from temporary

    std::cout << "5) vector push_back vs emplace_back:\n";
    std::vector<Track> v;
    v.reserve(4);
    v.push_back(Track{"tmp"}); // construct temp + move
    v.push_back(d);            // copy
    v.push_back(std::move(d)); // move
    v.emplace_back("inplace"); // construct directly inside the vector
    std::cout << "end\n";
}
