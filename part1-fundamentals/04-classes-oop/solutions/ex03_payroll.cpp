#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Employee {
public:
    explicit Employee(std::string name) : name_{std::move(name)} {}
    virtual ~Employee() = default;

    const std::string& name() const { return name_; }
    virtual double monthly_pay() const = 0;
    virtual std::string role() const = 0;

private:
    std::string name_;
};

class SalariedEmployee : public Employee {
public:
    SalariedEmployee(std::string name, double annual) : Employee{std::move(name)}, annual_{annual} {}
    double monthly_pay() const override { return annual_ / 12.0; }
    std::string role() const override { return "salaried"; }

private:
    double annual_;
};

class HourlyEmployee : public Employee {
public:
    HourlyEmployee(std::string name, double hours, double rate)
        : Employee{std::move(name)}, hours_{hours}, rate_{rate} {}

    double monthly_pay() const override {
        constexpr double kRegular = 160.0;
        if (hours_ <= kRegular) return hours_ * rate_;
        return kRegular * rate_ + (hours_ - kRegular) * rate_ * 1.5;
    }
    std::string role() const override { return "hourly"; }

private:
    double hours_, rate_;
};

class Manager : public SalariedEmployee {
public:
    Manager(std::string name, double annual, double bonus)
        : SalariedEmployee{std::move(name), annual}, bonus_{bonus} {}

    void add_report(const Employee& e) { reports_.push_back(&e); } // non-owning observers

    double monthly_pay() const override { return SalariedEmployee::monthly_pay() + bonus_; } // reuse base
    std::string role() const override { return std::format("manager of {}", reports_.size()); }

private:
    double bonus_;
    std::vector<const Employee*> reports_;
};

int main() {
    std::vector<std::unique_ptr<Employee>> staff;
    staff.push_back(std::make_unique<SalariedEmployee>("Ada", 96000));
    staff.push_back(std::make_unique<HourlyEmployee>("Linus", 170, 40));
    staff.push_back(std::make_unique<HourlyEmployee>("Grace", 120, 45));

    auto boss = std::make_unique<Manager>("Bjarne", 120000, 1500);
    for (const auto& e : staff) boss->add_report(*e);
    staff.push_back(std::move(boss)); // ownership moves into the vector

    double total = 0;
    std::cout << std::format("{:<10} {:<14} {:>10}\n", "name", "role", "pay");
    for (const auto& e : staff) {
        std::cout << std::format("{:<10} {:<14} {:>10.2f}\n", e->name(), e->role(), e->monthly_pay());
        total += e->monthly_pay();
    }
    std::cout << std::format("{:<25} {:>10.2f}\n", "TOTAL", total);
}
