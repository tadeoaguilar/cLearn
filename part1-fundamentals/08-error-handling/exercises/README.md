# 08 — Exercises: Error Handling

### Ex 1 — Config parser with `std::expected` ⭐⭐
Parse lines such as `port = 8080`, `# comment`, `host=localhost` into a
`std::map<std::string, std::string>`. Return
`std::expected<Config, ConfigError>`, where `ConfigError` holds a line number
and a message. Blank lines and `#` comments are allowed. A line without `=`,
or an empty key, is an error. Then write
`std::expected<int, ConfigError> get_int(const Config&, std::string_view key)`.
→ `solutions/ex01_config_parser.cpp`

### Ex 2 — An exception hierarchy mapped to HTTP status ⭐⭐
Create `AppError : std::runtime_error` with a virtual `int http_status() const`,
plus `ValidationError` (400), `NotFoundError` (404) and `ConflictError` (409).
Write `transfer(from, to, amount)` on a small in-memory account map that
throws the right error. In `main`, run several transfers and print
`HTTP <status>: <message>`. Unknown exceptions map to 500. This is exactly how
the CRUD API handles errors.
→ `solutions/ex02_http_errors.cpp`

### Ex 3 — Optional chaining ⭐
Given `std::map<std::string, std::map<std::string, int>> population`
(country → city → people), write
`std::optional<int> population_of(country, city)` and print the results with
`value_or`. Then use `.transform` to format the number in millions.
→ `solutions/ex03_optional_chain.cpp`

### Ex 4 — All-or-nothing inventory ⭐⭐⭐
`Inventory` holds `std::map<std::string, int>` of stock. Implement
`apply(const std::vector<Op>& ops)`, where `Op` is `{item, delta}`. If any op
would make stock negative, throw, and leave the inventory **completely
unchanged** (the strong guarantee). Show that a failing batch has no effect.
→ `solutions/ex04_strong_guarantee.cpp`
