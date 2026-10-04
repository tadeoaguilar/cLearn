#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>

struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct BinOp {
    char op;
    ExprPtr lhs, rhs;
};

struct Expr {
    std::variant<double, BinOp> node;
};

ExprPtr num(double v) { return std::make_unique<Expr>(Expr{v}); }
ExprPtr bin(char op, ExprPtr l, ExprPtr r) { return std::make_unique<Expr>(Expr{BinOp{op, std::move(l), std::move(r)}}); }

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

double evaluate(const Expr& e) {
    return std::visit(overloaded{
                          [](double v) { return v; },
                          [](const BinOp& b) {
                              double l = evaluate(*b.lhs), r = evaluate(*b.rhs);
                              switch (b.op) {
                                  case '+': return l + r;
                                  case '-': return l - r;
                                  case '*': return l * r;
                                  case '/':
                                      if (r == 0) throw std::domain_error("division by zero");
                                      return l / r;
                              }
                              throw std::invalid_argument(std::string("unknown op ") + b.op);
                          },
                      },
                      e.node);
}

std::string to_string(const Expr& e) {
    return std::visit(overloaded{
                          [](double v) {
                              auto s = std::to_string(v);
                              s.erase(s.find_last_not_of('0') + 1); // trim trailing zeros
                              if (s.back() == '.') s.pop_back();
                              return s;
                          },
                          [](const BinOp& b) { return "(" + to_string(*b.lhs) + " " + b.op + " " + to_string(*b.rhs) + ")"; },
                      },
                      e.node);
}

int main() {
    // (2 + 3) * (10 - 4) / 3
    auto expr = bin('/', bin('*', bin('+', num(2), num(3)), bin('-', num(10), num(4))), num(3));
    std::cout << to_string(*expr) << " = " << evaluate(*expr) << '\n';

    auto bad = bin('/', num(1), bin('-', num(2), num(2)));
    try {
        evaluate(*bad);
    } catch (const std::exception& e) {
        std::cout << to_string(*bad) << " -> error: " << e.what() << '\n';
    }
}
