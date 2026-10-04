#include <iostream>
#include <numbers>
#include <string>
#include <variant>
#include <vector>

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

// ---- Part 1: closed set of shapes ----
struct Circle { double r; };
struct Rect { double w, h; };
struct Triangle { double base, height; };
using Shape = std::variant<Circle, Rect, Triangle>;

double area(const Shape& s) {
    return std::visit(overloaded{
                          [](const Circle& c) { return std::numbers::pi * c.r * c.r; },
                          [](const Rect& r) { return r.w * r.h; },
                          [](const Triangle& t) { return 0.5 * t.base * t.height; },
                      },
                      s);
}

// ---- Part 2: a game state machine ----
namespace state {
struct MainMenu {};
struct Playing { int level; int score; };
struct Paused { Playing saved; };
struct GameOver { int final_score; };
} // namespace state
using GameState = std::variant<state::MainMenu, state::Playing, state::Paused, state::GameOver>;

enum class Event { Start, Score, Pause, Resume, Die };

GameState on_event(const GameState& current, Event e) {
    using namespace state;
    return std::visit(overloaded{
                          [&](const MainMenu&) -> GameState {
                              if (e == Event::Start) return Playing{1, 0};
                              return current;
                          },
                          [&](const Playing& p) -> GameState {
                              switch (e) {
                                  case Event::Score: return Playing{p.level, p.score + 100};
                                  case Event::Pause: return Paused{p};
                                  case Event::Die:   return GameOver{p.score};
                                  default:           return current;
                              }
                          },
                          [&](const Paused& p) -> GameState { return e == Event::Resume ? GameState{p.saved} : current; },
                          [&](const GameOver&) -> GameState { return e == Event::Start ? GameState{Playing{1, 0}} : current; },
                      },
                      current);
}

std::string describe(const GameState& s) {
    return std::visit(overloaded{
                          [](const state::MainMenu&) { return std::string{"MainMenu"}; },
                          [](const state::Playing& p) { return "Playing(level " + std::to_string(p.level) + ", score " + std::to_string(p.score) + ")"; },
                          [](const state::Paused& p) { return "Paused(score " + std::to_string(p.saved.score) + ")"; },
                          [](const state::GameOver& g) { return "GameOver(" + std::to_string(g.final_score) + ")"; },
                      },
                      s);
}

int main() {
    std::vector<Shape> shapes{Circle{1}, Rect{2, 3}, Triangle{4, 5}};
    for (const auto& s : shapes) std::cout << "area = " << area(s) << '\n';

    Shape s = Rect{1, 1};
    if (auto* r = std::get_if<Rect>(&s)) std::cout << "it's a rect " << r->w << "x" << r->h << '\n';
    std::cout << "index() = " << s.index() << ", sizeof(Shape) = " << sizeof(Shape) << " bytes, no heap\n\n";

    GameState g = state::MainMenu{};
    for (Event e : {Event::Start, Event::Score, Event::Score, Event::Pause, Event::Score, Event::Resume, Event::Die}) {
        g = on_event(g, e);
        std::cout << describe(g) << '\n';
    }
}
