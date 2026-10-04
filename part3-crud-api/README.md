# 15 — Project: Tasks API (REST CRUD in Modern C++)

A production-shaped HTTP JSON API for managing tasks, written in C++23.
It ties everything together: classes and interfaces (ch. 4), RAII and smart
pointers (ch. 5), the STL (ch. 6), templates (ch. 7), error handling (ch. 8),
concurrency (ch. 11), CMake and testing (ch. 12), JSON (ch. 13) and
PostgreSQL (ch. 14).

```
curl -X POST localhost:8080/tasks -H 'Content-Type: application/json' \
     -d '{"title":"Learn C++","priority":"high","due_date":"2026-12-31"}'
{"id":1,"title":"Learn C++","description":"","status":"todo","priority":"high",
 "due_date":"2026-12-31","created_at":"2026-10-03T14:35:26Z","updated_at":"2026-10-03T14:35:26Z"}
```

## Quick start

**Option 1: everything in Docker** (no local toolchain needed)
```bash
cd part3-crud-api
docker compose up --build          # API on :8080, PostgreSQL on :5433
./scripts/smoke_test.sh            # in another terminal: hits every endpoint
```

**Option 2: native, in-memory storage** (just CMake + a C++23 compiler)
```bash
cmake -S . -B build -DTASKS_WITH_POSTGRES=OFF
cmake --build build
./build/tasks_tests                # 23 test cases
./build/tasks_api                  # http://localhost:8080
```

**Option 3: native, with PostgreSQL**
```bash
brew install libpq                 # macOS (Linux: apt install libpq-dev)
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix libpq)"
cmake --build build
docker compose up -d db
DATABASE_URL=postgresql://tasks:tasks@localhost:5433/tasks ./build/tasks_api
TEST_DATABASE_URL=postgresql://tasks:tasks@localhost:5433/tasks ./build/tasks_tests   # + Postgres contract tests
```

## Endpoints

| Method | Path | Body | Success | Errors |
|--------|------|------|---------|--------|
| GET | `/health` | — | 200 `{"status":"ok","storage":"postgres"}` | 503 |
| GET | `/tasks` | — | 200 `{"items":[...],"total":N,"limit":20,"offset":0}` | 422 |
| POST | `/tasks` | `{title, description?, status?, priority?, due_date?}` | 201 + `Location` | 400, 422 |
| GET | `/tasks/{id}` | — | 200 task | 404 |
| PUT | `/tasks/{id}` | full task (same as POST) | 200 task | 400, 404, 422 |
| PATCH | `/tasks/{id}` | any subset; `"due_date": null` clears it | 200 task | 400, 404, 422 |
| DELETE | `/tasks/{id}` | — | 204 | 404 |

**List query parameters:** `status` (`todo|in_progress|done`), `priority`
(`low|medium|high`), `q` (case-insensitive search in title/description), `sort`
(`created_at|updated_at|due_date|priority|title`), `order` (`asc|desc`, default
`desc`), `limit` (1–100, default 20), `offset` (≥ 0).

**Error format** (always JSON):
```json
{"error": {"code": "validation_failed", "message": "the request contains invalid fields",
           "details": {"title": "must not be empty", "colour": "unknown field"}}}
```
| Status | `code` | When |
|--------|--------|------|
| 400 | `malformed_json` | body isn't valid JSON |
| 404 | `not_found` | unknown id or route |
| 422 | `validation_failed` | valid JSON, invalid content (wrong types, unknown fields, rules) |
| 500 | `internal_error` | a bug: details are logged, never sent to clients |
| 503 | `unavailable` | the database is unreachable or the pool is exhausted |

## Architecture

```
                 ┌────────────────────────────────────────────┐
  HTTP request → │ http/routes.cpp     parse params & JSON,   │  knows HTTP + JSON
                 │ http/json_mapping   map errors → status    │
                 └───────────────────┬────────────────────────┘
                                     ▼
                 ┌────────────────────────────────────────────┐
                 │ service/task_service  validation, rules    │  pure C++, no I/O
                 └───────────────────┬────────────────────────┘
                                     ▼  depends on an INTERFACE
                 ┌────────────────────────────────────────────┐
                 │ repository/task_repository.hpp (abstract)  │
                 └──────────┬─────────────────────┬───────────┘
                            ▼                     ▼
          MemoryTaskRepository           PgTaskRepository ── infra/connection_pool
          (std::map + shared_mutex)      (libpqxx, SQL)      infra/migrations
```
`main.cpp` is the **composition root**. It reads the config, picks the
repository, and wires everything together. Dependencies point **inward**:
the domain knows nothing about HTTP or SQL.

## Project layout

```
part3-crud-api/
├── CMakeLists.txt            tasks_core lib, tasks_pg lib, tasks_api exe, tasks_tests
├── cmake/Dependencies.cmake  FetchContent: nlohmann/json, cpp-httplib, libpqxx, doctest
├── Dockerfile, compose.yaml  multi-stage image; API + PostgreSQL stack
├── scripts/smoke_test.sh     curl-based end-to-end check
├── src/
│   ├── main.cpp              composition root, signals, server start
│   ├── app/                  config (env vars), logger
│   ├── domain/               Task, NewTask, TaskPatch, ListQuery, errors, validation helpers
│   ├── service/              TaskService: business rules
│   ├── repository/           interface + memory + PostgreSQL implementations
│   ├── infra/                connection pool, migrations
│   └── http/                 routes, JSON mapping
├── tests/                    domain, service, JSON, repository contract, HTTP end-to-end
└── docs/                     the learning guide for this project ↓
```

## Configuration (environment variables)

| Variable | Default | Meaning |
|----------|---------|---------|
| `PORT` | `8080` | listen port |
| `HOST` | `0.0.0.0` | listen address |
| `STORAGE` | `postgres` if `DATABASE_URL` is set, else `memory` | storage backend |
| `DATABASE_URL` | — | e.g. `postgresql://tasks:tasks@localhost:5433/tasks` |
| `DB_POOL_SIZE` | `8` | max PostgreSQL connections |
| `HTTP_THREADS` | `8` | HTTP worker threads |
| `LOG_LEVEL` | `info` | `debug`, `info`, `warn`, `error` |

## Learning guide

Read these in order. Each one explains *why* the code looks the way it does.

1. [HTTP & REST primer](docs/01-http-rest-primer.md): methods, status codes, idempotency, resource design
2. [Architecture walkthrough](docs/02-architecture.md): layers, a request's journey, threading model
3. [Persistence layer](docs/03-persistence.md): repository pattern, connection pool, migrations, SQL safety
4. [Testing strategy](docs/04-testing.md): unit, contract and end-to-end tests
5. [Deployment](docs/05-deployment.md): Docker, configuration, health checks, graceful shutdown
6. [Exercises with solutions](docs/06-exercises.md): extend the API (stats, auth, optimistic locking, tags...)
