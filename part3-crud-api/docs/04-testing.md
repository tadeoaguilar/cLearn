# 4. Testing Strategy

```
          ▲  few, slow, realistic
          │   scripts/smoke_test.sh        curl against a running container
          │   tests/test_http.cpp          real server + real client, in-process
          │   tests/test_repository_contract.cpp   memory + (optionally) PostgreSQL
          │   tests/test_service.cpp       business rules with the in-memory repo
          │   tests/test_json_mapping.cpp  parsing/serialization
          ▼   tests/test_domain.cpp        pure functions
             many, fast, focused
```

Run them:
```bash
./build/tasks_tests                                   # everything except Postgres
./build/tasks_tests -tc="*HTTP*"                      # filter by test-case name
./build/tasks_tests -s                                # show successful assertions too
TEST_DATABASE_URL=postgresql://tasks:tasks@localhost:5433/tasks ./build/tasks_tests
ctest --test-dir build --output-on-failure            # through CMake
```

## Unit tests (domain, JSON, service)
Pure functions and the service tested with `MemoryTaskRepository`. They're
fast (milliseconds) and deterministic. They pin down rules such as "title is
trimmed", "an empty patch is rejected", "`null` clears `due_date`" and "every
invalid field is reported at once".

`TEST_CASE_FIXTURE(Fixture, ...)` gives every test a fresh repository and service.

## Contract tests (repositories)
`run_contract(TaskRepository&)` holds the behavior **any** repository must
have. It runs against the in-memory repo always, and against PostgreSQL when
`TEST_DATABASE_URL` is set. This keeps the fake honest. Without it, the fake
used by the service tests could drift from what the real database does.

doctest's `SUBCASE`s re-run the enclosing test case from the top for each
subcase. That's why the Postgres test case truncates the table at its start:
each subcase gets a clean table.

## End-to-end HTTP tests
`TestServer` binds to a **random free port** (`bind_to_any_port`), runs
`listen_after_bind` on a `jthread`, and waits with `wait_until_ready()`. Tests
then use `httplib::Client` like any external client, checking status codes,
headers (`Location`) and JSON bodies. One test fires 80 POSTs from 8 threads to
make sure the in-memory repo is thread-safe. Run it under ThreadSanitizer:

```bash
cmake -S . -B build-tsan -DTASKS_WITH_POSTGRES=OFF -DCMAKE_CXX_FLAGS="-fsanitize=thread -g"
cmake --build build-tsan && ./build-tsan/tasks_tests
```

## Smoke test
`scripts/smoke_test.sh` runs curl against a deployed instance and checks
status codes. Use it after `docker compose up` or a deployment.

## What to test when you add a feature
1. Domain/service unit tests for the new rules.
2. Extend the contract if the repository interface changed.
3. An HTTP test for the status codes and the JSON shape.
4. Run the whole suite with Postgres before calling it done. The `LIMIT`/`OFFSET`
   bug in [03-persistence.md](03-persistence.md) shows why.
