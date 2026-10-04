# 2. Architecture Walkthrough

## The dependency rule

```
   http  ───►  service  ───►  repository (interface)  ◄───  memory / postgres
     │            │                    │
     └────────────┴────────► domain ◄──┘
```
Arrows mean "knows about". The **domain** (`Task`, `NewTask`, errors) knows
nothing. The **service** knows the domain and the repository *interface*. Only
`main.cpp` knows the concrete classes. This gives us:

- **Testability**: `TaskService` tests use `MemoryTaskRepository`, so they're fast and need no DB.
- **Replaceability**: swap PostgreSQL for SQLite by writing one class.
- **Clarity**: SQL lives in exactly one file, and HTTP status codes in exactly one function.

## Module tour

| File | Responsibility | C++ techniques |
|------|----------------|----------------|
| `domain/task.hpp` | entity & value types, enums, validation helpers | `enum class`, `std::optional`, defaulted `operator==`, `constexpr` |
| `domain/errors.hpp` | `ValidationError`, `NotFoundError`, `StorageUnavailableError` | exception hierarchy (ch. 8) |
| `service/task_service.*` | normalize + validate, call the repository | designated initializers, references to interfaces |
| `repository/task_repository.hpp` | the storage contract | pure virtual interface (ch. 4) |
| `repository/memory_task_repository.*` | in-memory store | `std::map`, `std::shared_mutex`, ranges `drop`/`take` |
| `repository/pg_task_repository.*` | SQL | libpqxx, params, transactions, generic lambda helper |
| `infra/connection_pool.*` | share N connections across threads | RAII lease, `condition_variable`, move-only types |
| `infra/migrations.*` | evolve the schema | advisory locks, transactional DDL |
| `http/json_mapping.*` | JSON ⇄ domain, strict parsing | nlohmann/json, template member function |
| `http/routes.*` | endpoints, error → status mapping, access log | lambdas, higher-order function `guarded`, `thread_local` |
| `app/config.*` | env vars → `Config` | `std::from_chars`, validation |
| `app/logger.*` | thread-safe log lines | `std::format_string`, variadic templates, mutex |
| `main.cpp` | composition root, lifecycle | `unique_ptr<Interface>`, `jthread`, `sigwait` |

## A request's journey: `PATCH /tasks/7 {"status":"done"}`

1. **httplib** accepts the TCP connection, and a worker thread from its thread pool
   parses the request.
2. The **pre-routing handler** stores the start time in a `thread_local`.
3. The router matches `/tasks/:id` and calls our lambda, wrapped by `guarded(...)`.
4. `parse_id` → `7`. `parse_body` checks `Content-Type`, then `json::parse`.
5. `patch_from_json` builds a `TaskPatch{.status = Done}`. It **collects every
   problem** (types, unknown fields) and throws one `ValidationError`.
6. `TaskService::patch` validates the business rules (non-empty patch, title
   length, date format), then calls `repo_.update(7, patch)`.
7. `PgTaskRepository::update` builds `UPDATE tasks SET updated_at = now(), status = $1
   WHERE id = $2 RETURNING ...`, leases a connection from the pool, runs it in a
   transaction and maps the row back to a `Task`. If no row comes back,
   it returns `nullopt`, and the service throws `NotFoundError`.
8. Back in the route: `to_json(task)` → `send(res, 200, ...)`.
9. If anything threw, `guarded` caught it and produced the right status + JSON error.
10. The **logger** callback prints `PATCH /tasks/7 -> 200 (1.12 ms)`.

## Error handling strategy

```
 domain / service       throw ValidationError, NotFoundError
 repository (pg)        catches pqxx::broken_connection → throws StorageUnavailableError
 http::guarded          catches ALL of the above → 4xx/5xx JSON
 server exception hdlr  last-resort safety net → 500
```
Exceptions are a good fit for a server: an error deep in the stack unwinds
straight to the one place that knows how to report it, and RAII releases the
DB connection lease and rolls back the transaction on the way.

Notice the two-phase validation. **Shape** errors (types, unknown fields) are
caught while parsing JSON, and **rule** errors (empty title, bad date) in the
service. A request with shape errors gets those reported first. Merging both
phases into one report is a nice small exercise.

## Threading model

- httplib runs each request on one of `HTTP_THREADS` worker threads. Many
  requests run **concurrently**.
- `MemoryTaskRepository` protects its map with a `std::shared_mutex`: lists and
  finds share the lock, and writes take it exclusively.
- `PgTaskRepository` holds no mutable state. Each call leases its own
  `pqxx::connection`, which is not thread-safe, from the `ConnectionPool`. The
  pool's mutex is held only while picking a connection, never during a query.
- PostgreSQL handles concurrency between requests (MVCC, row locks, constraints).
- `Logger` serializes writes to stderr with a mutex, and formats outside the lock.

Rule of thumb: **size the DB pool ≥ HTTP threads**. Otherwise threads queue for
connections. If the pool is exhausted for 5 s, `acquire` throws, and clients get 503.

## Graceful shutdown

`main` blocks `SIGINT`/`SIGTERM` in every thread, and a dedicated `std::jthread`
waits for them with `sigwait`. On a signal it calls `server.stop()`: `listen()`
returns, in-flight requests finish, and destructors run in reverse order
(server → service → repository → pool, which closes connections). Docker and
Kubernetes send `SIGTERM` before killing a container, so this matters in production.
