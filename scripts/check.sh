#!/usr/bin/env bash
# Compiles every standalone example and solution in parts 1, 2 and 5 with the
# system compiler. No CMake required. Use this to verify your toolchain or
# after editing many files.
#
#   ./scripts/check.sh            # compile only
#   ./scripts/check.sh --run      # compile and run each program (non-interactive ones)
#   ./scripts/check.sh 06-stl     # only files whose path contains "06-stl"
#   ./scripts/check.sh --run 11-  # combine both
#
# Programs that read from stdin get /dev/null as input when run.
# Files containing the marker 'check.sh: expected-warning' warn on purpose.
# Files that #include <pqxx/...> are skipped unless pkg-config finds libpqxx.
# .c files (part 5) are compiled as C17 with $CC. A line '// build: also-compile X'
# adds X (for example an assembly .S file next to it) to that program.

set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${TMPDIR:-/tmp}/clearn-check"
CXX="${CXX:-clang++}"
STD="${STD:-c++23}"
CC="${CC:-clang}"
CSTD="${CSTD:-c17}"
RUN=0
FILTER=""
for arg in "$@"; do
  if [[ "$arg" == "--run" ]]; then RUN=1; else FILTER="$arg"; fi
done

mkdir -p "$OUT"
pass=0; fail=0; skip=0
failed=()

have_pqxx=0
TIMEOUT=""
command -v timeout >/dev/null && TIMEOUT="timeout 10"
command -v gtimeout >/dev/null && TIMEOUT="gtimeout 10"
if command -v pkg-config >/dev/null && pkg-config --exists libpqxx 2>/dev/null; then have_pqxx=1; fi

while IFS= read -r src; do
  rel="${src#$ROOT/}"
  [[ -n "$FILTER" && "$rel" != *"$FILTER"* ]] && continue
  # Only files directly inside examples/ or solutions/; nested folders are CMake projects.
  [[ "$rel" =~ /(examples|solutions)/[^/]+\.(cpp|c)$ ]] || continue
  if grep -q '#include <pqxx' "$src"; then
    if [[ $have_pqxx -eq 0 ]]; then
      echo "SKIP  $rel (libpqxx not found)"; skip=$((skip+1)); continue
    fi
    extra=$(pkg-config --cflags --libs libpqxx)
  else
    extra=""
  fi
  exe="$OUT/$(echo "$rel" | tr '/' '_' | sed -E 's/\.(cpp|c)$//')"
  if [[ "$src" == *.c ]]; then
    compiler=("$CC" -std="$CSTD")
    while IFS= read -r partner; do
      extra="$extra $(dirname "$src")/$partner"
    done < <(sed -n 's|^// build: also-compile *||p' "$src")
  else
    compiler=("$CXX" -std="$STD")
  fi
  # shellcheck disable=SC2086
  if "${compiler[@]}" -Wall -Wextra -Wpedantic -pthread "$src" -o "$exe" $extra 2>"$exe.log"; then
    if [[ -s "$exe.log" ]] && ! grep -q 'check.sh: expected-warning' "$src"; then echo "WARN  $rel"; sed 's/^/      /' "$exe.log" | head -10; else echo "OK    $rel"; fi
    pass=$((pass+1))
    if [[ $RUN -eq 1 ]]; then
      (cd "$OUT" && $TIMEOUT "$exe" </dev/null >"$exe.out" 2>&1) || echo "      (non-zero exit or timeout running $rel)"
    fi
  else
    echo "FAIL  $rel"; sed 's/^/      /' "$exe.log" | head -20
    fail=$((fail+1)); failed+=("$rel")
  fi
done < <(find "$ROOT/part1-fundamentals" "$ROOT/part2-persistence" "$ROOT/part5-c-lowlevel" \
           -path '*/examples/*.cpp' -o -path '*/solutions/*.cpp' \
           -o -path '*/examples/*.c' -o -path '*/solutions/*.c' | sort)

echo
echo "passed: $pass  failed: $fail  skipped: $skip"
[[ $fail -eq 0 ]]
