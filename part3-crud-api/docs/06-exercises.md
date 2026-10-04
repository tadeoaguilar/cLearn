# 6. Exercises: Extend the Tasks API

Each exercise is a realistic feature request. Try it on a branch:

```bash
git switch -c my-exercises        # (run `git init` at the repo root first if needed)
```

**Reference solutions**: all six are implemented, tested (memory + PostgreSQL,
clang + GCC) and packaged as one patch:

```bash
cd part3-crud-api
patch -p1 --dry-run < docs/exercise-solutions.patch   # check
patch -p1 < docs/exercise-solutions.patch             # apply   (undo: patch -R -p1 < ...)
cmake --build build && ./build/tasks_tests            # 29 test cases (30 with TEST_DATABASE_URL)
```
The patch adds `tests/test_exercises.cpp`, plus contract tests for the new
repository methods. The sections below explain each solution's design. Open
the solution blocks only after you've tried.

---

## Ex 1 — `GET /tasks/stats` ⭐⭐

Return dashboard counters:
```json
{"total": 12, "by_status": {"todo": 5, "in_progress": 3, "done": 4},
 "by_priority": {"low": 2, "medium": 6, "high": 4}, "overdue": 2}
```
`overdue` = has a `due_date` before today and isn't `done`. Include zero counts.

**Hints:** add `virtual TaskStats stats(std::string_view today) = 0;` to the
repository. Why pass `today` in instead of reading the clock inside? Watch the
route registration order.

<details><summary>Solution notes</summary>

- **Domain**: `struct TaskStats { total; std::map<TaskStatus, int64> by_status; std::map<Priority, int64> by_priority; overdue; }`.
- **Clock injection**: the service computes today with
  `std::format("{:%F}", floor<days>(system_clock::now()))` and passes it down.
  The repository is then deterministic, and the contract test can use
  `"2026-06-15"` as "today".
- **Memory**: one pass over the map, under a `shared_lock`. ISO dates compare
  correctly as strings.
- **PostgreSQL**: one table scan:
  ```sql
  SELECT status, priority, count(*),
         count(*) FILTER (WHERE due_date < $1::date AND status <> 'done')
  FROM tasks GROUP BY status, priority
  ```
  then fold the rows into the maps in C++.
- **Routing gotcha**: httplib tries routes in registration order, and
  `/tasks/:id` matches `/tasks/stats` (id = "stats" → 422). Register
  `/tasks/stats` **before** `/tasks/:id`.
- **JSON**: loop over every enum value so zero counts appear.
</details>

---

## Ex 2 — API-key authentication ⭐⭐

If `API_KEY` is set, every request except `/health` must send
`Authorization: Bearer <key>`. Otherwise, respond `401` with a
`WWW-Authenticate: Bearer` header and the usual JSON error.

<details><summary>Solution notes</summary>

- `RouteOptions{api_key, rate_limit_rps, rate_limit_burst}` is passed to
  `register_routes`. `Config` reads `API_KEY`, and `main` warns when it's empty.
- httplib's `set_pre_routing_handler` runs before routing. Returning
  `HandlerResponse::Handled` short-circuits the request. Only **one**
  pre-routing handler exists, so timing, auth and rate limiting share it.
- **Constant-time comparison**: `a == b` stops at the first differing byte, and
  that timing difference can leak the key. Compare every byte instead:
  ```cpp
  unsigned char diff = 0;
  for (std::size_t i = 0; i < a.size(); ++i) diff |= a[i] ^ b[i];
  return diff == 0;
  ```
- `/health` stays public so load balancers can probe it.
- Production note: use real identity (OAuth2/OIDC JWTs, mTLS) and always TLS.
  A shared key is fine for service-to-service calls or a personal API.
</details>

---

## Ex 3 — Optimistic locking with ETag / If-Match ⭐⭐⭐

Two users open task 7, both edit it, and the second save silently overwrites
the first one: a **lost update**. Fix it:
- every task has a `version` that is incremented on each update,
- responses carry `ETag: "<version>"`,
- `PUT`/`PATCH` with `If-Match: "<version>"` succeed only if the version still
  matches. Otherwise they return **412 Precondition Failed**,
- without `If-Match`, the last write wins (backwards compatible).

<details><summary>Solution notes</summary>

- **Migration 004**: `ALTER TABLE tasks ADD COLUMN version INT NOT NULL DEFAULT 1`.
  A new migration, never an edit of 001.
- **Repository signature** now distinguishes the two failure modes with C++23 `std::expected`:
  ```cpp
  virtual std::expected<Task, UpdateError> update(TaskId, const TaskPatch&,
                                                  std::optional<int> expected_version) = 0;
  enum class UpdateError { NotFound, VersionConflict };
  ```
- **The SQL is atomic**: the version check is part of the `UPDATE`, so there's
  no read-then-write race:
  ```sql
  UPDATE tasks SET ..., version = version + 1 WHERE id = $n AND version = $m RETURNING ...
  ```
  If no row comes back, `SELECT EXISTS(...)` in the same transaction tells
  NotFound apart from VersionConflict.
- **Service**: maps `VersionConflict` → `PreconditionFailedError` (domain) →
  `412` (HTTP, in `guarded`).
- **HTTP**: `set_etag(res, task)` on GET/POST/PUT/PATCH. `if_match_version(req)`
  accepts `"3"` and `W/"3"`. Garbage → 422.
- Try it:
  ```bash
  curl -si localhost:8080/tasks/1 | grep ETag                                   # ETag: "1"
  curl -s -X PATCH localhost:8080/tasks/1 -H 'If-Match: "1"' -H 'Content-Type: application/json' -d '{"title":"A"}'
  curl -s -X PATCH localhost:8080/tasks/1 -H 'If-Match: "1"' -H 'Content-Type: application/json' -d '{"title":"B"}'  # 412
  ```
</details>

---

## Ex 4 — Bulk complete ⭐⭐

`POST /tasks/bulk-complete {"ids": [1, 2, 3]}` marks the tasks done **in one
transaction** and returns `{"updated": <count>}`. It ignores ids that don't
exist or are already done. Limit: 100 ids. Duplicates are fine.

<details><summary>Solution notes</summary>

- The service validates (non-empty, ≤ 100, positive) and de-duplicates with
  `std::ranges::sort` + `std::ranges::unique` + `erase`.
- PostgreSQL: **one** statement with an array parameter, so there's no N+1
  round trips:
  ```sql
  UPDATE tasks SET status = 'done', updated_at = now(), version = version + 1
  WHERE id = ANY($1) AND status <> 'done'
  ```
  libpqxx converts a `std::vector<TaskId>` into a PostgreSQL array, and
  `affected_rows()` is the count.
- Memory: one `unique_lock` for the whole batch, so readers never see half of it.
- It's a `POST` on a sub-resource ("an action"). That's a pragmatic REST
  compromise for operations that don't map to a single resource.
</details>

---

## Ex 5 — One validation report ⭐

Today `{"title":"  ","priority":"urgent"}` reports only `priority`, because
shape errors are thrown before the business rules run. Make the response list
**all** problems at once.

<details><summary>Solution notes</summary>

`Reader::finish_with_rules(value)` calls the service's `tasks::validate(value)`
inside a `try`, and merges its field errors with `problems_.try_emplace(...)`.
`try_emplace` keeps the *shape* error when a field has both, because a
non-string title has no meaningful length. The service still validates again
later. That's harmless and keeps the service safe when it's used without HTTP.
</details>

---

## Ex 6 — Rate limiting ⭐⭐⭐

Limit each client IP to `RATE_LIMIT_RPS` requests per second, with bursts of up
to 20. Excess requests get **429 Too Many Requests** with `Retry-After: 1`.
`/health` is never limited.

<details><summary>Solution notes</summary>

**Token bucket**: each client has a bucket of `burst` tokens. Each request
spends one, and tokens refill continuously at `rate` per second:
```cpp
double elapsed = duration<double>(now - b.last).count();
b.tokens = std::min(burst_, b.tokens + elapsed * rate_);
b.last = now;
if (b.tokens < 1.0) return false;     // → 429
b.tokens -= 1.0;
```
- One `std::mutex` around an `unordered_map<std::string, Bucket>`, keyed by
  `req.remote_addr`. The critical section is a few arithmetic operations.
- `steady_clock`, never `system_clock`: wall-clock jumps (NTP) would mint or
  steal tokens.
- The limiter lives in a `shared_ptr` captured by the pre-routing lambda.
  httplib copies handlers, so all copies must share one limiter.
- Production concerns: evict idle buckets (memory grows with distinct IPs),
  trust `X-Forwarded-For` only from your own proxy, and with several replicas
  keep the buckets in Redis or let the API gateway do the limiting.
</details>

---

## More ideas (no reference solution)

- **Tags**: `tags` and `task_tags` tables (many-to-many), `GET /tasks?tag=work`,
  `PUT /tasks/{id}/tags`. Practice JOINs, `= ANY`, and the transactional replace
  of a set.
- **Keyset pagination**: `GET /tasks?after_id=120&limit=20`, using
  `WHERE id < $1 ORDER BY id DESC`. Measure it against `OFFSET` with 1M rows.
- **Soft delete**: a `deleted_at` column. DELETE sets it, lists hide such rows,
  and `POST /tasks/{id}/restore` brings a task back.
- **Idempotency keys**: `POST /tasks` with `Idempotency-Key: <uuid>` returns
  the same response when retried.
- **SQLite repository**: implement `TaskRepository` with SQLite and make it
  pass `run_contract`.
- **Structured logging + request ids**: JSON log lines with an `X-Request-Id`
  echoed back to clients.
- **OpenAPI**: write `openapi.yaml` for the API and serve it at `/openapi.yaml`.
