#include <iostream>
#include <string>
#include <utility>

void take(std::string& s)        { std::cout << "  lvalue ref:       " << s << '\n'; }
void take(const std::string& s)  { std::cout << "  const lvalue ref: " << s << '\n'; }
void take(std::string&& s)       { std::cout << "  rvalue ref:       " << s << " (may steal)\n"; }

std::string make() { return "temporary"; }

int main() {
    std::string name = "named";
    const std::string fixed = "const named";

    take(name);              // lvalue
    take(fixed);             // const lvalue
    take(make());            // rvalue (temporary)
    take(std::string{"x"});  // rvalue
    take(std::move(name));   // std::move = cast to rvalue

    // std::move alone does nothing to the object:
    std::string s = "still here";
    auto&& r = std::move(s);  // just a reference; no move happened
    (void)r;
    std::cout << "after std::move without a receiver: s = \"" << s << "\"\n";

    // The move happens when a move constructor is chosen:
    std::string t = std::move(s);
    std::cout << "after real move: s = \"" << s << "\" (valid but unspecified), t = \"" << t << "\"\n";

    // Moving a const object silently COPIES
    const std::string c = "const";
    std::string d = std::move(c); // calls the copy ctor: const string&& can't bind to string&&
    std::cout << "moved-from const is unchanged: \"" << c << "\"\n";
}
