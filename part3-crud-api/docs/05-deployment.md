# 5. Deployment

## The Docker image (multi-stage)

```dockerfile
FROM ubuntu:24.04 AS build      # compilers, headers, CMake: ~1 GB, thrown away
... cmake --build build && ./build/tasks_tests   # the build FAILS if tests fail
FROM ubuntu:24.04               # runtime: libpq5 + our binary only
COPY --from=build /src/build/tasks_api /usr/local/bin/tasks_api
USER app                        # never run as root
HEALTHCHECK CMD curl -fsS http://localhost:8080/health
```
- `-static-libstdc++ -static-libgcc` bakes the C++ runtime into the binary,
  so the runtime image doesn't need a matching libstdc++.
- Layers are ordered from least to most frequently changed, for cache hits.
- `.dockerignore` keeps local `build/` folders out of the build context.

```bash
docker compose up --build -d
docker compose logs -f api
docker compose exec db psql -U tasks -d tasks -c 'select id, title, status from tasks'
docker compose down          # keep data   |   docker compose down -v   # delete the volume too
```

## Twelve-factor configuration
All configuration comes from environment variables (`app/config.cpp`), so the
same image runs in dev, CI and production. Invalid values fail **at startup**
with a clear message, rather than at the first request.

## Health checks
`GET /health` runs `SELECT 1` through the pool. Load balancers and orchestrators
use it to route traffic only to healthy instances, and to restart broken ones.
Production systems often split this into **liveness** ("is the process alive")
and **readiness** ("can it serve: DB reachable, migrations done").

## Graceful shutdown
`SIGTERM` → `server.stop()` → in-flight requests complete → destructors close the
DB connections. Kubernetes waits `terminationGracePeriodSeconds` (30 s by
default) before `SIGKILL`.

## Logging
One line per request on stderr, with an ISO timestamp, level, method, path,
status and latency. Containers collect stderr. For production, consider
**structured JSON logs** and a **request id** header propagated through log lines.

## Production checklist (beyond this project)
- [ ] TLS: terminate at a reverse proxy (nginx, Caddy, a cloud LB), or build httplib with OpenSSL
- [ ] Authentication & authorization (see exercise 2)
- [ ] Rate limiting (exercise 6)
- [ ] Metrics: request counts, latencies, pool usage (Prometheus `/metrics`)
- [ ] Database backups & tested restores (`pg_dump`, PITR)
- [ ] Secrets from a secret manager, not committed `compose.yaml` files
- [ ] CI: build with clang **and** gcc, run tests against Postgres, run sanitizers
- [ ] Request size limits and timeouts (we set a 1 MiB body limit)
