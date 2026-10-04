#include <iostream>
#include <memory>
#include <numbers>
#include <string>
#include <vector>

class Shape {
public:
    virtual ~Shape() = default;              // polymorphic base → virtual dtor
    virtual double area() const = 0;         // pure virtual: must be overridden
    virtual double perimeter() const = 0;
    virtual std::string name() const { return "shape"; } // has a default

    // Non-virtual "template method" built on the virtual ones
    void describe() const {
        std::cout << name() << ": area=" << area() << " perimeter=" << perimeter() << '\n';
    }
};

class Circle final : public Shape { // final: nobody can derive from Circle
public:
    explicit Circle(double r) : r_{r} {}
    double area() const override { return std::numbers::pi * r_ * r_; }
    double perimeter() const override { return 2 * std::numbers::pi * r_; }
    std::string name() const override { return "circle"; }

private:
    double r_;
};

class Rectangle : public Shape {
public:
    Rectangle(double w, double h) : w_{w}, h_{h} {}
    double area() const override { return w_ * h_; }
    double perimeter() const override { return 2 * (w_ + h_); }
    std::string name() const override { return "rectangle"; }

protected: // accessible to derived classes
    double w_, h_;
};

class Square : public Rectangle {
public:
    explicit Square(double side) : Rectangle{side, side} {} // call the base constructor
    std::string name() const override { return "square"; }
};

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(1.0));
    shapes.push_back(std::make_unique<Rectangle>(2.0, 3.0));
    shapes.push_back(std::make_unique<Square>(2.0));

    double total = 0;
    for (const auto& s : shapes) {
        s->describe(); // dynamic dispatch picks the right area()/name()
        total += s->area();
    }
    std::cout << "total area = " << total << "\n\n";

    // Slicing: copying a Square into a Rectangle keeps only the Rectangle part
    Square sq{4};
    Rectangle sliced = sq;
    std::cout << "sliced.name() = " << sliced.name() << " (the Square part was sliced off)\n";
    const Shape& ref = sq; // a reference keeps the dynamic type
    std::cout << "ref.name()    = " << ref.name() << '\n';

    // dynamic_cast: ask "is this Shape actually a Circle?"
    for (const auto& s : shapes)
        if (auto* c = dynamic_cast<Circle*>(s.get())) std::cout << "found a circle with area " << c->area() << '\n';

    std::cout << "sizeof(Circle) = " << sizeof(Circle) << " (double + hidden vptr)\n";
}
