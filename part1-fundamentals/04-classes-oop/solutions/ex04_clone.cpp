#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Shape {
public:
    virtual ~Shape() = default;
    virtual std::unique_ptr<Shape> clone() const = 0; // "virtual copy constructor"
    virtual void scale(double f) = 0;
    virtual std::string describe() const = 0;
};

class Circle : public Shape {
public:
    explicit Circle(double r) : r_{r} {}
    std::unique_ptr<Shape> clone() const override { return std::make_unique<Circle>(*this); } // copy ctor
    void scale(double f) override { r_ *= f; }
    std::string describe() const override { return "Circle(r=" + std::to_string(r_) + ")"; }

private:
    double r_;
};

class Rect : public Shape {
public:
    Rect(double w, double h) : w_{w}, h_{h} {}
    std::unique_ptr<Shape> clone() const override { return std::make_unique<Rect>(*this); }
    void scale(double f) override { w_ *= f; h_ *= f; }
    std::string describe() const override { return "Rect(" + std::to_string(w_) + "x" + std::to_string(h_) + ")"; }

private:
    double w_, h_;
};

using Shapes = std::vector<std::unique_ptr<Shape>>;

Shapes deep_copy(const Shapes& src) {
    Shapes out;
    out.reserve(src.size());
    for (const auto& s : src) out.push_back(s->clone());
    return out;
}

void print_shapes(const std::string& label, const Shapes& shapes) {
    std::cout << label << ":";
    for (const auto& s : shapes) std::cout << ' ' << s->describe();
    std::cout << '\n';
}

int main() {
    Shapes original;
    original.push_back(std::make_unique<Circle>(1));
    original.push_back(std::make_unique<Rect>(2, 3));

    // Shapes copy = original; // ERROR: unique_ptr is not copyable
    Shapes copy = deep_copy(original);

    for (auto& s : original) s->scale(2);

    print_shapes("original (scaled)", original);
    print_shapes("copy (untouched) ", copy);
}
