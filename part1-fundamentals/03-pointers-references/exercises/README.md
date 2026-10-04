# 03 — Exercises: Pointers, References & Views

### Ex 1 — Reverse with two pointers ⭐
Write `void reverse(int* begin, int* end)` that reverses `[begin, end)` in place
by walking two pointers toward each other. Then write
`void reverse(std::span<int>)` that calls the first one. Test it on a C array
and on a `std::vector`.
→ `solutions/ex01_reverse.cpp`

### Ex 2 — Find the max element's address ⭐
`const int* find_max(std::span<const int> values)` returns a pointer to the
largest element, or `nullptr` for an empty span. In `main`, print the value
*and* its index (hint: subtracting pointers gives a distance).
→ `solutions/ex02_find_max.cpp`

### Ex 3 — Trim & key=value parsing ⭐⭐
1. `std::string_view trim(std::string_view s)` removes leading and trailing whitespace.
2. Parse `"  host = localhost ;port=5432;  user= admin "` into key/value pairs
   (as `std::string_view`s) and print them. No `std::string` allocations
   while parsing!
→ `solutions/ex03_trim_parse.cpp`

### Ex 4 — Matrix with a flat buffer ⭐⭐
Store a `rows x cols` matrix in a single `std::vector<double>` (row-major).
Write `std::span<double> row(std::vector<double>& data, int cols, int r)`
that returns a view of row `r`, plus a `transpose` function. Print the matrices.
→ `solutions/ex04_matrix.cpp`

### Ex 5 — Out-parameters: pointer vs reference ⭐⭐
Write `bool parse_int(std::string_view s, int& out)` and
`bool parse_int(std::string_view s, int* out)` (the pointer version accepts
`nullptr` and then only validates). Use `std::from_chars` from `<charconv>`.
Explain in a comment why returning `std::optional<int>` (ch. 8) is nicer.
→ `solutions/ex05_out_params.cpp`
