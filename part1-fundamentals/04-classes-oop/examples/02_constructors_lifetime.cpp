// Watch constructors and destructors run. Objects die in reverse order.
#include <iostream>
#include <string>
#include <vector>

class Tracer {
public:
    explicit Tracer(std::string name) : name_{std::move(name)} {
        ++alive_;
        std::cout << "  ctor   " << name_ << "  (alive=" << alive_ << ")\n";
    }
    Tracer(const Tracer& other) : name_{other.name_ + "-copy"} {
        ++alive_;
        std::cout << "  copy   " << name_ << "  (alive=" << alive_ << ")\n";
    }
    Tracer& operator=(const Tracer& other) {
        std::cout << "  assign " << name_ << " = " << other.name_ << '\n';
        name_ = other.name_ + "-assigned";
        return *this;
    }
    ~Tracer() {
        --alive_;
        std::cout << "  dtor   " << name_ << "  (alive=" << alive_ << ")\n";
    }

    static int alive() { return alive_; }

private:
    std::string name_;
    static inline int alive_ = 0; // shared by all Tracers
};

class Holder {
public:
    // Members are built in DECLARATION order (first_, then second_),
    // regardless of the order in this list.
    Holder() : first_{"member-1"}, second_{"member-2"} { std::cout << "  Holder body runs last\n"; }
    ~Holder() { std::cout << "  ~Holder body runs first, then members die\n"; }

private:
    Tracer first_;
    Tracer second_;
};

int main() {
    std::cout << "== scope order ==\n";
    {
        Tracer a{"a"};
        Tracer b{"b"};
    } // b dies, then a

    std::cout << "== copy vs assignment ==\n";
    {
        Tracer x{"x"};
        Tracer y = x; // copy CONSTRUCTION (y is new)
        y = x;        // copy ASSIGNMENT (y already exists)
    }

    std::cout << "== members ==\n";
    { Holder h; }

    std::cout << "== heap ==\n";
    Tracer* p = new Tracer{"heap"}; // lives until delete (ch. 5 shows the better way)
    std::cout << "  alive now: " << Tracer::alive() << '\n';
    delete p;

    std::cout << "== vector growth copies elements ==\n";
    std::vector<Tracer> v;
    v.reserve(2); // without reserve you'd see extra copies on reallocation
    v.emplace_back("v1"); // constructed in place
    v.emplace_back("v2");
    std::cout << "== end of main ==\n";
}
