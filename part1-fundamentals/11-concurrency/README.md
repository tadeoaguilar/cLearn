# 11 — Concurrency

> Goal: run work in parallel **safely**. Threads, data races, mutexes,
> condition variables, atomics, futures, and a thread pool. Our HTTP server
> handles requests on many threads, and Unreal has a game thread, a render
> thread and task graphs, so this matters later.

## 1. Threads

```cpp
#include <thread>
std::jthread t([] { do_work(); });   // C++20: joins automatically in its destructor
std::thread  u(do_work, arg1);       // C++11: you MUST join() or detach() before it's destroyed
u.join();                            // wait for it to finish
std::thread::hardware_concurrency(); // number of hardware threads
```
Arguments are **copied** into the thread. Use `std::ref(x)` to pass a reference.
Make sure the referenced object outlives the thread.

## 2. Data races: the core problem

A **data race** happens when two threads access the same memory at the same
time, at least one access is a write, and there is no synchronization. It is
**undefined behavior**. You won't just get "a wrong number". The compiler may
reorder or cache values in ways that break everything.

```cpp
int counter = 0;
// 4 threads each doing ++counter 100000 times → result is usually NOT 400000
```
`++counter` is read-modify-write: three steps that can interleave between threads.

Detect races with **ThreadSanitizer**: `-fsanitize=thread`.

## 3. Mutexes and locks

```cpp
std::mutex m;
{
    std::lock_guard lock(m);   // locks now, unlocks at end of scope (RAII!)
    ++counter;
}
std::unique_lock lk(m);        // movable, can unlock/relock, needed for condition_variable
std::scoped_lock both(m1, m2); // locks several mutexes without deadlock
std::shared_mutex rw;          // many readers OR one writer
std::shared_lock read(rw);     // reader lock
```

**Deadlock**: thread A holds m1 and waits for m2, while B holds m2 and waits
for m1. Avoid it by always locking in the same order, using `std::scoped_lock`
for several mutexes at once, and never calling unknown code while holding a lock.

**Keep critical sections short.** Copy the data out under the lock, then
process it outside.

## 4. Condition variables: waiting for something

```cpp
std::mutex m;
std::condition_variable cv;
std::queue<Job> q;

// consumer
std::unique_lock lk(m);
cv.wait(lk, [&] { return !q.empty(); });  // ALWAYS use the predicate (spurious wakeups)
auto job = q.front(); q.pop();

// producer
{ std::lock_guard lk(m); q.push(job); }
cv.notify_one();
```

## 5. Atomics

For single variables, `std::atomic<T>` gives lock-free, race-free operations:

```cpp
std::atomic<int> counter{0};
counter.fetch_add(1);          // or ++counter
counter.load(); counter.store(5);
std::atomic<bool> stop{false};
```
Memory orders (`memory_order_relaxed`, `acquire`, `release`, `seq_cst`) are an
advanced topic. The default `seq_cst` is correct, and you can optimize later.

## 6. Futures and async: results from other threads

```cpp
std::future<int> f = std::async(std::launch::async, [] { return compute(); });
int result = f.get();          // waits; rethrows if the task threw

std::promise<int> p;           // the producing side, set manually
std::future<int> fut = p.get_future();
std::jthread t([&p] { p.set_value(42); });
```
`std::packaged_task` wraps a callable so that calling it fulfills a future. We
use it to build a thread pool.

## 7. `std::jthread` and cooperative cancellation (C++20)

```cpp
std::jthread worker([](std::stop_token st) {
    while (!st.stop_requested()) { tick(); }
});
worker.request_stop();   // also called automatically by the destructor
```

## 8. Parallel algorithms

```cpp
#include <execution>
std::sort(std::execution::par, v.begin(), v.end());
std::reduce(std::execution::par_unseq, v.begin(), v.end());
```
Support varies. GCC needs TBB, and Apple's libc++ has incomplete support.

## 9. Design guidelines

1. **Share nothing** when you can. Give each thread its own data and combine the results.
2. Pass messages through thread-safe queues instead of sharing state.
3. Protect each piece of shared state with exactly one mutex, and keep them together in a class.
4. Prefer high-level tools (futures, thread pools, parallel algorithms) to raw threads.
5. Test with `-fsanitize=thread`.

## Examples
| File | Shows |
|------|-------|
| `examples/01_threads.cpp` | creating/joining threads, `jthread`, passing args, `std::ref` |
| `examples/02_race_and_mutex.cpp` | a real data race vs mutex vs atomic (with timings) |
| `examples/03_producer_consumer.cpp` | condition variable + thread-safe queue |
| `examples/04_async_future.cpp` | `std::async`, futures, promises, exceptions across threads |
| `examples/05_thread_pool.cpp` | a reusable thread pool returning futures |
| `examples/06_stop_token.cpp` | cooperative cancellation with `jthread` |
