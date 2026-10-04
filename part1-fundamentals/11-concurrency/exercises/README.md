# 11 — Exercises: Concurrency

Build every solution once with `-fsanitize=thread` to confirm there are no races.

### Ex 1 — Parallel word count ⭐⭐
Generate a large text (e.g. 2 000 000 words drawn from a list of 20). Split it
into N chunks, have N threads each count words in a **thread-local**
`unordered_map`, then merge. Compare the time against the single-threaded
version, and verify the counts match.
→ `solutions/ex01_parallel_wordcount.cpp`

### Ex 2 — Deadlock-free bank transfers ⭐⭐⭐
`struct Account { std::mutex m; int balance; };`. 8 threads perform 10 000
random transfers between 5 accounts. Lock both accounts with `std::scoped_lock`,
so there's no deadlock even when A→B and B→A run at the same time. Skip
transfers that would overdraw. Verify that the total money is conserved.
→ `solutions/ex02_bank_transfers.cpp`

### Ex 3 — Pipeline of stages ⭐⭐⭐
Use a `BlockingQueue` (from example 03) to build a 3-stage pipeline: a
generator thread produces 1..20, a "square" thread squares the numbers, and a
"sink" thread sums them. Each stage closes its output queue when its input is
exhausted. Expected sum: 2870.
→ `solutions/ex03_pipeline.cpp`

### Ex 4 — Read-mostly cache with `shared_mutex` ⭐⭐
A `SettingsCache` with `get(key)` (shared lock: many concurrent readers) and
`set(key, value)` (exclusive lock). Run 6 reader threads and 1 writer thread
for ~200 ms. Count the reads and writes, and make sure no reader ever sees a
half-written value. Store `std::string` values and check an invariant.
→ `solutions/ex04_shared_mutex_cache.cpp`
