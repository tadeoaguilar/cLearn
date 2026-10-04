# 14 — Exercises: PostgreSQL

Every solution creates its own `ex_*` tables, so the solutions can run in any
order. Build them like the examples (see the chapter README).

### Ex 1 — Library catalog ⭐⭐
Create `ex_authors` and `ex_books` (with a foreign key and `ON DELETE CASCADE`).
Insert 3 authors and 6 books **in one transaction**, using `RETURNING id` to
link the books to their authors. Then:
1. list every book with its author name (`JOIN`), ordered by author then year
2. print the number of books per author, including authors with 0 books (`LEFT JOIN`, `GROUP BY`)
3. delete one author, and show that their books disappeared with the cascade
→ `solutions/ex01_library_catalog.cpp`

### Ex 2 — Safe dynamic search ⭐⭐⭐
Implement
```cpp
struct BookQuery {
    std::optional<std::string> title_contains;
    std::optional<int> min_year;
    std::string sort_by = "title";   // "title" | "year" | "price"
    bool descending = false;
    int limit = 10, offset = 0;
};
std::vector<Book> search(pqxx::connection&, const BookQuery&);
```
Build the `WHERE` clause dynamically, but with **every value as a parameter**.
`sort_by` must be checked against a **whitelist**, because column names can't
be parameters. Unknown values throw `std::invalid_argument`. Clamp `limit` to
1..100. Show that `sort_by = "price; DROP TABLE ex_books"` is rejected.
→ `solutions/ex02_safe_search.cpp`

### Ex 3 — No overselling under concurrency ⭐⭐⭐
A product has `stock = 10`. Start 8 threads, each with **its own connection**,
and have each try to buy 3 units.
1. **Naive**: `SELECT stock`, check it in C++, then `UPDATE ... SET stock = <computed>`.
   Count the successful purchases. You'll typically sell more than 10!
2. **Correct (A)**: one atomic statement,
   `UPDATE ... SET stock = stock - $1 WHERE id = $2 AND stock >= $1`, then check `affected_rows()`.
3. **Correct (B)**: `SELECT ... FOR UPDATE`, then update in the same transaction.
Print the units sold and the final stock for each strategy.
→ `solutions/ex03_no_overselling.cpp`

### Ex 4 — Repository contract tests ⭐⭐⭐
Define a `NoteRepository` interface (`create`, `get`, `list(limit, offset)`,
`update`, `remove`) with Postgres and in-memory implementations. Write **one**
function, `run_contract_tests(NoteRepository&)`, that checks the behavior
every implementation must have: ids are unique, get after create returns
equal data, update/remove of a missing id return false, pagination works.
Run it against both implementations. This technique keeps fakes honest, and
the API's tests use the same idea.
→ `solutions/ex04_repository_contract.cpp`
