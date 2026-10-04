# 14 — Databases with PostgreSQL and libpqxx

> Goal: store and query data in a real relational database from C++. You'll
> review SQL, connect with **libpqxx**, run safe parameterized queries, use
> transactions, map errors, and organize the code with the **repository
> pattern** and **migrations**, ready for the CRUD API.

## 1. Why a database server?

Chapter 13 ended with the limits of files. PostgreSQL gives you:
- **ACID transactions**:
  - **A**tomic: all or nothing.
  - **C**onsistent: constraints always hold.
  - **I**solated: concurrent transactions don't see each other's half-done work.
  - **D**urable: committed data survives crashes, thanks to the write-ahead log from ch. 13 ex. 4.
- **Concurrency**: many clients read and write safely at the same time.
- **Queries**: filter, join, aggregate and sort with SQL, using indexes for speed.
- **Constraints**: `NOT NULL`, `UNIQUE`, `CHECK`, `FOREIGN KEY`. The database
  guards the invariants even when application code has bugs.

```
 ┌──────────────┐   TCP 5432    ┌──────────────────────┐
 │  C++ program │◄─────────────►│  PostgreSQL server   │
 │  libpqxx     │  SQL + params │  tables, indexes,    │
 │  └─ libpq (C)│◄──── rows ────│  WAL, MVCC           │
 └──────────────┘               └──────────────────────┘
```

## 2. Running PostgreSQL

```bash
docker compose up -d db                                # from the repo root
docker compose exec db psql -U clearn -d clearn        # interactive SQL shell
```
Connection string (URI form): `postgresql://user:password@host:port/dbname`.
Our programs read it from the `DATABASE_URL` environment variable, falling back
to `postgresql://clearn:clearn@localhost:5432/clearn`.

Useful `psql` commands: `\dt` (tables), `\d tasks` (describe), `\x` (expanded
output), `\timing`, `\q`.

## 3. SQL refresher

```sql
-- DDL: define structure
CREATE TABLE authors (
    id         BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name       TEXT NOT NULL UNIQUE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT now()
);
CREATE TABLE books (
    id        BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    author_id BIGINT NOT NULL REFERENCES authors(id) ON DELETE CASCADE,
    title     TEXT NOT NULL CHECK (length(title) > 0),
    year      INT,
    price     NUMERIC(10,2) NOT NULL DEFAULT 0
);
CREATE INDEX books_author_idx ON books(author_id);

-- DML: manipulate data
INSERT INTO authors(name) VALUES ('Stroustrup') RETURNING id;
SELECT title, year FROM books WHERE year >= 2010 ORDER BY year DESC LIMIT 10 OFFSET 0;
UPDATE books SET price = price * 0.9 WHERE year < 2000;
DELETE FROM books WHERE id = 7;

-- JOIN + aggregate
SELECT a.name, count(b.id) AS books, avg(b.price)::numeric(10,2) AS avg_price
FROM authors a LEFT JOIN books b ON b.author_id = a.id
GROUP BY a.name HAVING count(b.id) > 0 ORDER BY books DESC;

-- Upsert
INSERT INTO settings(key, value) VALUES ('theme', 'dark')
ON CONFLICT (key) DO UPDATE SET value = EXCLUDED.value;
```
`sql/` has a schema, seed data and practice queries: run
`docker compose exec -T db psql -U clearn -d clearn < part2-persistence/14-postgresql/sql/01_schema.sql`.

### Types you'll use most
| PostgreSQL | C++ (libpqxx `as<T>()`) |
|------------|-------------------------|
| `BIGINT`, `INT` | `std::int64_t`, `int` |
| `TEXT`, `VARCHAR` | `std::string` |
| `BOOLEAN` | `bool` |
| `NUMERIC(10,2)` | `std::string` or `double` (money: prefer integer cents or a decimal type) |
| `TIMESTAMPTZ` | `std::string` (ISO 8601); parse into `std::chrono` if needed |
| `JSONB` | `std::string` → nlohmann::json |
| nullable column | `std::optional<T>` |

## 4. libpqxx essentials

**libpqxx** is the official C++ client. It sits on top of **libpq**, PostgreSQL's C library.

```cpp
#include <pqxx/pqxx>

pqxx::connection conn{"postgresql://clearn:clearn@localhost:5432/clearn"};  // RAII: closes in dtor

pqxx::work tx{conn};                          // BEGIN
pqxx::result r = tx.exec(
    "SELECT id, title FROM books WHERE year >= $1 ORDER BY id",
    pqxx::params{2010});                       // $1 is bound safely, never concatenated
for (const auto& row : r) {
    auto id = row["id"].as<std::int64_t>();
    auto title = row["title"].as<std::string>();
}
// Typed iteration: one tuple per row
for (auto [id, title] : tx.query<std::int64_t, std::string>("SELECT id, title FROM books"))
    std::cout << id << ' ' << title << '\n';

auto count = tx.query_value<int>("SELECT count(*) FROM books");
auto new_id = tx.exec("INSERT INTO authors(name) VALUES ($1) RETURNING id",
                      pqxx::params{"Meyers"}).one_row()[0].as<std::int64_t>();
tx.commit();                                   // COMMIT. Without it, the destructor ROLLS BACK
```

| Type | Use |
|------|-----|
| `pqxx::connection` | one TCP session. **Not thread-safe**: one per thread, or use a pool |
| `pqxx::work` | a read-write transaction (`BEGIN` … `COMMIT`/rollback) |
| `pqxx::read_transaction` | read-only transaction |
| `pqxx::nontransaction` | autocommit each statement (no rollback) |
| `pqxx::result` / `pqxx::row` / `pqxx::field` | query results |
| `pqxx::params` | positional parameters `$1, $2, ...` |

### NULLs
```cpp
std::optional<int> year = row["year"].as<std::optional<int>>();   // NULL → nullopt
tx.exec("UPDATE books SET year = $1 WHERE id = $2", pqxx::params{std::optional<int>{}, id}); // writes NULL
```
`row["year"].as<int>()` on a NULL **throws**.

### Prepared statements
The server parses and plans the statement once, and you execute it many times:
```cpp
conn.prepare("insert_book", "INSERT INTO books(author_id, title, year) VALUES ($1, $2, $3)");
tx.exec(pqxx::prepped{"insert_book"}, pqxx::params{author_id, "Title", 2024});
```

## 5. SQL injection: never build SQL with string concatenation

```cpp
// ❌ user_input = "x'; DROP TABLE books; --"
tx.exec("SELECT * FROM books WHERE title = '" + user_input + "'");
// ✅ parameters are sent separately from the SQL text. They can never become code.
tx.exec("SELECT * FROM books WHERE title = $1", pqxx::params{user_input});
```
Identifiers (table and column names, `ORDER BY` direction) **can't** be
parameters. Pick them from a **whitelist** in code (exercise 2).

## 6. Transactions in depth

```cpp
pqxx::work tx{conn};
tx.exec("UPDATE accounts SET balance = balance - $1 WHERE id = $2", pqxx::params{amount, from});
tx.exec("UPDATE accounts SET balance = balance + $1 WHERE id = $2", pqxx::params{amount, to});
tx.commit();      // both or neither
```
- If anything throws before `commit()`, the `pqxx::work` destructor rolls back. **RAII again.**
- After a statement fails, PostgreSQL aborts the transaction. You must start a new one.
- **Isolation levels**: `READ COMMITTED` (default), `REPEATABLE READ`, `SERIALIZABLE`.
  The stricter levels may throw `pqxx::serialization_failure`. **Retry** the
  whole transaction when that happens.
- **Row locks**: `SELECT ... FOR UPDATE` locks the rows you're about to change.
  That prevents lost updates such as "two buyers grab the last item".
- Keep transactions **short**. Never wait for user input or network calls inside one.

## 7. Errors

```
pqxx::failure
 ├── pqxx::broken_connection        → retry / 503
 ├── pqxx::sql_error  (.sqlstate(), .query())
 │    ├── pqxx::integrity_constraint_violation
 │    │    ├── pqxx::unique_violation        (23505) → 409 Conflict
 │    │    ├── pqxx::foreign_key_violation   (23503) → 400/409
 │    │    ├── pqxx::not_null_violation      (23502) → 400
 │    │    └── pqxx::check_violation         (23514) → 400
 │    ├── pqxx::syntax_error / undefined_table  → bug: 500
 │    └── pqxx::transaction_rollback → serialization_failure, deadlock_detected → retry
 └── pqxx::conversion_error          → bad as<T>() on a field
```
The repository layer should translate these into **domain errors**
(`DuplicateEmail`, `NotFound`), so the rest of the app doesn't depend on pqxx.

## 8. The repository pattern

```cpp
class UserRepository {                       // domain-facing interface
public:
    virtual ~UserRepository() = default;
    virtual User create(const NewUser&) = 0;
    virtual std::optional<User> find(std::int64_t id) = 0;
    virtual std::vector<User> list(int limit, int offset) = 0;
    virtual bool update(const User&) = 0;
    virtual bool remove(std::int64_t id) = 0;
};
class PgUserRepository : public UserRepository { ... uses pqxx ... };
class InMemoryUserRepository : public UserRepository { ... std::map ... };  // tests, demos
```
Business logic depends only on the interface. You can test it without a
database, and swap storage engines later. Example 05 shows the full pattern,
and it's the backbone of part 3.

## 9. Migrations

Schemas evolve. A **migration** is an ordered, versioned schema change
(`001_create_users`, `002_add_last_login`...). The app or a tool records the
applied versions in a `schema_migrations` table, and applies the missing ones
in transactions at startup. Never edit a migration that has already shipped.
Add a new one. Example 06 implements a minimal migrator. Real projects use
tools such as Flyway, dbmate, sqitch or Atlas.

## 10. Performance tips

- Add **indexes** for columns in `WHERE`, `JOIN` and `ORDER BY`, and check
  them with `EXPLAIN ANALYZE`.
- Fetch only the columns you need, and **paginate** (`LIMIT/OFFSET`, or
  keyset pagination: `WHERE id > $last ORDER BY id LIMIT 50`).
- Batch inserts in **one transaction**, or use `COPY` (`pqxx::stream_to`) for bulk loads.
- Reuse connections. Opening one costs a TCP and auth round trip, so servers use a **pool**.
- Avoid **N+1 queries** (one query per item in a loop). Use a `JOIN` or `WHERE id = ANY($1)`.

## Building the examples

These need libpqxx, which our CMake downloads and builds, plus the system **libpq**:

```bash
# Option A — macOS native
brew install libpq
cmake -S . -B build -DCLEARN_BUILD_POSTGRES=ON -DCMAKE_PREFIX_PATH="$(brew --prefix libpq)"
cmake --build build --target ch14_ex_02_crud_basics
docker compose up -d db && ./build/part2-persistence/14-postgresql/ch14_ex_02_crud_basics

# Option B — no local installs: use the dev container (Linux, g++-14, libpq)
docker compose run --rm dev bash -c \
  "cmake -S . -B build-docker -G Ninja -DCLEARN_BUILD_POSTGRES=ON && cmake --build build-docker --target ch14_ex_02_crud_basics \
   && ./build-docker/part2-persistence/14-postgresql/ch14_ex_02_crud_basics"
```

## Files
| File | Shows |
|------|-------|
| `sql/01_schema.sql`, `02_seed.sql`, `03_queries.sql` | practice schema + queries for psql |
| `examples/db_common.hpp` | reads `DATABASE_URL`, shared helpers |
| `examples/01_connect.cpp` | connecting, server info, simple queries |
| `examples/02_crud_basics.cpp` | create/read/update/delete with params, NULLs, RETURNING |
| `examples/03_transactions.cpp` | commit/rollback, constraints, `FOR UPDATE`, retries |
| `examples/04_prepared_injection.cpp` | injection demo, prepared statements, batch vs single inserts |
| `examples/05_repository.cpp` | interface + Postgres + in-memory implementations, error mapping |
| `examples/06_migrations.cpp` | a minimal versioned migration runner |
