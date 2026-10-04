#include <iostream>
#include <variant>
#include <vector>

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

struct KeyPress { char key; };
struct MouseClick { int x, y; int button; };
struct Resize { int w, h; };
struct Quit {};
using Event = std::variant<KeyPress, MouseClick, Resize, Quit>;

int main() {
    std::vector<Event> queue{KeyPress{'w'}, KeyPress{'a'}, MouseClick{100, 200, 1}, Resize{1920, 1080},
                             KeyPress{'s'}, Quit{}, KeyPress{'x'}};

    int keys = 0, clicks = 0, resizes = 0;
    bool running = true;
    for (const auto& ev : queue) {
        if (!running) break;
        std::visit(overloaded{
                       [&](const KeyPress& k) { ++keys; std::cout << "key '" << k.key << "'\n"; },
                       [&](const MouseClick& m) { ++clicks; std::cout << "click b" << m.button << " at (" << m.x << ',' << m.y << ")\n"; },
                       [&](const Resize& r) { ++resizes; std::cout << "resize to " << r.w << 'x' << r.h << '\n'; },
                       [&](const Quit&) { running = false; std::cout << "quit\n"; },
                   },
                   ev);
    }
    std::cout << "keys=" << keys << " clicks=" << clicks << " resizes=" << resizes << " (the 'x' after Quit was ignored)\n";
}
