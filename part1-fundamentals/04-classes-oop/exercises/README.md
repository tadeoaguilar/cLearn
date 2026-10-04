# 04 — Exercises: Classes & OOP

### Ex 1 — Fraction ⭐⭐
Implement `class Fraction` with numerator and denominator `long long`.
- Always stored **normalized**: reduced by the gcd, denominator > 0. A zero
  denominator throws `std::invalid_argument`.
- Operators `+ - * /` (implemented via `+= -= *= /=`), `==`, `<=>` and `<<`.
- `double to_double() const`.
Verify that `1/2 + 1/3 == 5/6` and that `2/4 == 1/2`.
→ `solutions/ex01_fraction.cpp`

### Ex 2 — Clock time ⭐⭐
`class Time` stores seconds since midnight, and its invariant is
`0 <= s < 86400`. Provide `Time(int h, int m, int s)`, which throws if out of
range, plus `hours()`, `minutes()`, `seconds()`, `Time& operator+=(int seconds)`
(which wraps around midnight), `to_string()` (`"HH:MM:SS"`) and comparisons.
→ `solutions/ex02_time.cpp`

### Ex 3 — Payroll ⭐⭐
Abstract base `Employee` with `name()` and a pure virtual `double monthly_pay() const`.
Derived classes:
- `SalariedEmployee` (annual salary / 12)
- `HourlyEmployee` (hours × rate, with 1.5× overtime beyond 160 h)
- `Manager : SalariedEmployee` (salary + bonus, plus a list of reports)

Store everyone in `std::vector<std::unique_ptr<Employee>>`, print a payroll and the total.
→ `solutions/ex03_payroll.cpp`

### Ex 4 — Virtual copy (clone) ⭐⭐⭐
You can't copy a `std::vector<std::unique_ptr<Shape>>` directly. Add
`virtual std::unique_ptr<Shape> clone() const = 0;` to a Shape hierarchy and
write `std::vector<std::unique_ptr<Shape>> deep_copy(const std::vector<std::unique_ptr<Shape>>&)`.
Prove the copy is independent by mutating (e.g. `scale(2)`) the original.
→ `solutions/ex04_clone.cpp`
