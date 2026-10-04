#!/usr/bin/env bash
# Exercises every endpoint with curl and checks the status codes.
#   ./scripts/smoke_test.sh                 # against http://localhost:8080
#   BASE=http://localhost:9000 ./scripts/smoke_test.sh
set -euo pipefail
BASE="${BASE:-http://localhost:8080}"
pass=0; fail=0

expect() { # expect <code> <description> <curl args...>
  local want="$1" desc="$2"; shift 2
  local out code
  out=$(curl -s -w '\n%{http_code}' "$@")
  code=$(tail -n1 <<<"$out")
  BODY=$(sed '$d' <<<"$out")
  if [[ "$code" == "$want" ]]; then echo "ok   $code $desc"; pass=$((pass+1));
  else echo "FAIL $code (want $want) $desc"; echo "     $BODY"; fail=$((fail+1)); fi
}
json() { curl -s -H 'Content-Type: application/json' "$@"; }

expect 200 "health"                 "$BASE/health"
expect 201 "create task"            -X POST "$BASE/tasks" -H 'Content-Type: application/json' -d '{"title":"Smoke test","priority":"high","due_date":"2026-12-31"}'
ID=$(sed -E 's/.*"id":([0-9]+).*/\1/' <<<"$BODY")
expect 200 "get task $ID"           "$BASE/tasks/$ID"
expect 200 "patch task"             -X PATCH "$BASE/tasks/$ID" -H 'Content-Type: application/json' -d '{"status":"done"}'
expect 200 "replace task"           -X PUT "$BASE/tasks/$ID" -H 'Content-Type: application/json' -d '{"title":"Replaced"}'
expect 200 "list with filters"      "$BASE/tasks?status=todo&sort=title&order=asc&limit=5"
expect 422 "validation error"       -X POST "$BASE/tasks" -H 'Content-Type: application/json' -d '{"title":""}'
expect 400 "malformed json"         -X POST "$BASE/tasks" -H 'Content-Type: application/json' -d '{oops'
expect 422 "bad query param"        "$BASE/tasks?limit=1000"
expect 204 "delete task"            -X DELETE "$BASE/tasks/$ID"
expect 404 "get deleted task"       "$BASE/tasks/$ID"
expect 404 "unknown route"          "$BASE/does-not-exist"

echo; echo "passed: $pass  failed: $fail"
[[ $fail -eq 0 ]]
