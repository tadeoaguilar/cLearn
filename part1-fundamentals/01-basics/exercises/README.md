# 01 — Exercises: Basics

Solve each one in your own file first, then compare with `../solutions/`.
Compile with: `clang++ -std=c++23 -Wall -Wextra file.cpp -o out && ./out`

### Ex 1 — Temperature table ⭐
Print a table converting Celsius to Fahrenheit from -20 °C to 40 °C in steps of
10. Use `F = C * 9/5 + 32`. Align the columns with `std::format`.
*Watch out:* `9/5` in integer math is `1`!
→ `solutions/ex01_temperature.cpp`

### Ex 2 — FizzBuzz ⭐
For 1..30 print `Fizz` for multiples of 3, `Buzz` for multiples of 5,
`FizzBuzz` for both, otherwise the number.
→ `solutions/ex02_fizzbuzz.cpp`

### Ex 3 — Statistics ⭐⭐
Read integers from standard input until EOF, or until input that isn't a
number. Print count, min, max, sum and the average with 2 decimals. If no
numbers were given, print a friendly message instead.
Test: `echo "4 8 15 16 23 42" | ./out`
→ `solutions/ex03_stats.cpp`

### Ex 4 — Palindromes ⭐⭐
Write `bool is_palindrome(const std::string&)` that ignores case and any
non-alphanumeric characters. `"A man, a plan, a canal: Panama"` → true.
Hint: `std::isalnum`, `std::tolower` from `<cctype>`, and cast `char` to
`unsigned char` before calling them.
→ `solutions/ex04_palindrome.cpp`

### Ex 5 — Primes ⭐⭐
Print all primes ≤ 100, 10 per line, using the **Sieve of Eratosthenes** with a
`std::vector<bool>`.
→ `solutions/ex05_primes.cpp`

### Ex 6 — Mini calculator ⭐⭐⭐
Given expressions like `"12 + 30"`, `"7 / 0"`, `"9 * 9"`, `"5 ^ 2"` stored in a
vector of strings, parse each one with `std::istringstream` into
`double op double`, evaluate it with a `switch`, and print the result. Report
division by zero and unknown operators as errors.
→ `solutions/ex06_calculator.cpp`
