# 1. HTTP & REST Primer

## HTTP in one picture

HTTP is a text protocol over TCP. A client sends a **request**, and the server
answers with a **response**:

```
POST /tasks HTTP/1.1                       HTTP/1.1 201 Created
Host: localhost:8080                       Content-Type: application/json
Content-Type: application/json             Location: /tasks/7
Content-Length: 21                         Content-Length: 180

{"title":"Learn C++"}                      {"id":7,"title":"Learn C++",...}
```

- **Method**: what to do (`GET`, `POST`, `PUT`, `PATCH`, `DELETE`).
- **Path + query**: which resource (`/tasks/7`, `/tasks?status=done`).
- **Headers**: metadata (`Content-Type`, `Authorization`, `Location`, `ETag`...).
- **Body**: the payload, JSON in our case.
- **Status code**: the outcome, as a number.

See the raw exchange yourself: `curl -v localhost:8080/tasks`.

## REST: resources + standard methods

REST models the API as **resources** (nouns) identified by URLs and manipulated
with the standard **methods** (verbs):

| Method | On `/tasks` (collection) | On `/tasks/7` (item) | Safe? | Idempotent? |
|--------|--------------------------|----------------------|-------|-------------|
| GET | list | read | ✅ | ✅ |
| POST | create (server assigns id) | — | ❌ | ❌ |
| PUT | — | replace entirely | ❌ | ✅ |
| PATCH | — | modify some fields | ❌ | ❌ in general (✅ for our "set field" patches) |
| DELETE | — | delete | ❌ | ✅ |

- **Safe**: doesn't change server state. Clients, caches and crawlers may call it freely.
- **Idempotent**: calling it N times has the same effect as calling it once.
  That's why retrying a timed-out `PUT` or `DELETE` is OK, but retrying a
  `POST` may create duplicates. (Exercise idea: `Idempotency-Key` headers.)

Our API returns `404` for a second `DELETE` of the same task. The *state* is
the same either way (the task is gone), which is what idempotency is about.

### PUT vs PATCH
- `PUT /tasks/7 {"title":"x"}` **replaces** the whole task. Omitted fields go
  back to their defaults.
- `PATCH /tasks/7 {"status":"done"}` changes **only** `status`.
- In PATCH, `"due_date": null` means "clear it", and an absent `due_date` means
  "leave it alone". That's why `TaskPatch::due_date` is
  `std::optional<std::optional<std::string>>`.

## Status codes we use

| Code | Meaning | Used when |
|------|---------|-----------|
| 200 OK | success with a body | GET, PUT, PATCH |
| 201 Created | resource created | POST (+ `Location` header) |
| 204 No Content | success, no body | DELETE |
| 400 Bad Request | can't even parse the request | malformed JSON |
| 404 Not Found | no such resource/route | unknown id |
| 405 Method Not Allowed | route exists, method doesn't | `POST /tasks/7` |
| 409 Conflict | state conflict | (exercise: optimistic locking uses 412) |
| 422 Unprocessable Content | well-formed but invalid | empty title, wrong types |
| 500 Internal Server Error | our bug | unhandled exception |
| 503 Service Unavailable | temporary | database down |

Families: **2xx** success, **4xx** the client's fault (don't retry unchanged),
**5xx** the server's fault (a retry might help).

## Designing good JSON APIs

- **Consistent naming**: `snake_case` fields, plural collection names (`/tasks`).
- **Consistent error shape**: clients parse `error.code` (stable, machine-readable),
  while `message` is for humans.
- **Validate strictly**: reject unknown fields and wrong types. Silently ignoring
  `"titel"` hides client bugs.
- **Pagination** on every list endpoint (`limit`, `offset`, `total`). Unbounded
  lists are a production incident waiting to happen.
- **Never leak internals**: no SQL errors or stack traces in responses. Log them
  server-side.
- **Dates**: ISO 8601 strings, in UTC (`2026-10-03T14:35:26Z`).
- **IDs**: opaque to clients. We use integers. UUIDs avoid guessable ids.

## Trying the API with curl

```bash
BASE=localhost:8080
curl -s $BASE/health
curl -s -X POST $BASE/tasks -H 'Content-Type: application/json' -d '{"title":"Read docs"}'
curl -s "$BASE/tasks?status=todo&sort=due_date&order=asc&limit=5" | python3 -m json.tool
curl -s -X PATCH $BASE/tasks/1 -H 'Content-Type: application/json' -d '{"status":"done"}'
curl -s -X DELETE -o /dev/null -w '%{http_code}\n' $BASE/tasks/1
```
GUI alternatives: Bruno, Insomnia, Postman, or the VS Code "REST Client"
extension, all of which accept the same requests.
