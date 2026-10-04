# 3. The Persistence Layer

## One interface, two implementations

```cpp
class TaskRepository {
public:
    virtual Task create(const NewTask&) = 0;
    virtual std::optional<Task> find(TaskId) = 0;
    virtual ListResult list(const ListQuery&) = 0;
    virtual std::optional<Task> update(TaskId, const TaskPatch&) = 0;
    virtual bool remove(TaskId) = 0;
    ...
};
```
"Not found" is an expected outcome, so it's a return value (`nullopt`/`false`).
Connection failures are exceptional, so they're exceptions. The **service** turns
"not found" into `NotFoundError`, because only it knows that, in context, it's an error.

## The schema (migrations)

`src/infra/migrations.cpp` holds an append-only list:

| Version | Change | Why |
|---------|--------|-----|
| 001 | `CREATE TABLE tasks` with `CHECK` constraints | the DB enforces the same rules as the service: defense in depth |
| 002 | index on `(status, created_at DESC)` | the default list query filters by status and sorts by date |
| 003 | generated column `priority_rank` | sorting the text `priority` would be alphabetical (high < low < medium) |

Startup runs `run_migrations`, which:
1. ensures `schema_migrations` exists,
2. for each migration: takes `pg_advisory_xact_lock` (several API replicas
   starting together won't race), skips it if recorded, otherwise runs it **and**
   records it in the same transaction. PostgreSQL DDL is transactional, so a
   failing migration leaves nothing half-applied.

Inspect it: `docker compose exec db psql -U tasks -c 'select * from schema_migrations'`.

## SQL safety

- **Every value is a parameter** (`$1`, `$2`, …), including the search string.
- `ORDER BY` columns **can't** be parameters, so `order_column(SortField)` maps a
  C++ enum to fixed SQL text. User input never reaches the SQL string.
- `LIKE` wildcards are escaped, so searching `100%` finds the literal text "100%".

## Building dynamic SQL

`list()` and `update()` assemble SQL from optional parts:

```cpp
std::string where = " WHERE true";      // so every condition can start with AND
pqxx::params params;
int n = 0;
auto placeholder = [&n] { return "$" + std::to_string(++n); };
if (q.status) { where += " AND status = " + placeholder(); params.append(to_db(*q.status)); }
```

### ⚠️ A real bug we hit while writing this project
The first version had:
```cpp
std::string page = " LIMIT " + placeholder() + " OFFSET " + placeholder();
```
On macOS (clang) it produced `LIMIT $1 OFFSET $2`. In the Linux container
(GCC) it produced `LIMIT $2 OFFSET $1`, and every page came back empty. **C++
doesn't specify the order in which the operands of `+` are evaluated.** Each
`placeholder()` has a side effect (`++n`), so the result depended on the
compiler. The PostgreSQL contract tests caught it immediately. The fix is two
statements, because statements *are* sequenced:
```cpp
std::string page = " LIMIT " + placeholder();
page += " OFFSET " + placeholder();
```
Lesson: never call functions with side effects on shared state twice in one
expression, and run your tests on more than one compiler.

## Transactions & consistency

- Each write is a single statement in a `pqxx::work`, so it's atomic.
- `list()` runs the `count(*)` and the page query in **one** `REPEATABLE READ`,
  read-only transaction, so `total` and `items` come from the same snapshot even
  while other requests are inserting.
- `update()` uses `UPDATE ... RETURNING`, so we write and read back in one round
  trip with no race in between.

## The connection pool

```cpp
auto lease = pool_.acquire();   // RAII: returned in ~Lease
pqxx::work tx{*lease};
```
- Fixed maximum size. Connections are opened lazily up to `DB_POOL_SIZE`.
- `acquire()` waits on a `condition_variable`, and throws
  `StorageUnavailableError` (→ 503) after 5 s.
- Dead connections (e.g. the DB restarted) are detected on release or acquire,
  and replaced.
- The slot is reserved under the lock, but the slow TCP connect happens
  **outside** it.

## Memory vs Postgres: same behavior

`tests/test_repository_contract.cpp` runs **one** set of behavioral tests
against both implementations (ID uniqueness, patch semantics, filtering,
sorting with NULLs last, LIKE escaping, pagination). If you add an
implementation, for example SQLite, it must pass the same contract.

The in-memory version emulates SQL semantics on purpose:
- `due_date` sorts with missing values last (`NULLS LAST`),
- ties break by `id` so pagination is stable,
- search is case-insensitive over title **or** description.
