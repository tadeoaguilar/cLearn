# cLearn — A Hands-On C++ Learning Repository

A complete, project-driven path from "I know another language" to writing a
persistent REST API in modern C++ and gameplay code in Unreal Engine 5, plus
the C underneath it all: manual memory management, hardware drivers and assembly.

Every chapter follows the same structure:

```
NN-topic/
├── README.md            ← concepts explained in depth, with diagrams & pitfalls
├── examples/            ← small, runnable, heavily-commented programs
├── exercises/README.md  ← problems to solve (try them first!)
└── solutions/           ← one complete, compiled solution per exercise
```

## Roadmap

### Part 1 — Fundamentals of Modern C++ (C++20 / C++23)
| # | Chapter | You will learn |
|---|---------|----------------|
| 01 | [Basics](part1-fundamentals/01-basics/README.md) | compilation model, types, variables, control flow, I/O |
| 02 | [Functions](part1-fundamentals/02-functions/README.md) | parameters, overloading, defaults, headers vs sources |
| 03 | [Pointers & References](part1-fundamentals/03-pointers-references/README.md) | addresses, `const`, references, arrays, `std::span` |
| 04 | [Classes & OOP](part1-fundamentals/04-classes-oop/README.md) | classes, constructors, inheritance, virtual, operators |
| 05 | [Memory & RAII](part1-fundamentals/05-memory-raii/README.md) | stack vs heap, RAII, `unique_ptr`, `shared_ptr`, rule of 0/3/5 |
| 06 | [STL Containers & Algorithms](part1-fundamentals/06-stl/README.md) | `vector`, `map`, `unordered_map`, iterators, algorithms |
| 07 | [Templates & Generic Programming](part1-fundamentals/07-templates/README.md) | function/class templates, concepts |
| 08 | [Error Handling](part1-fundamentals/08-error-handling/README.md) | exceptions, `std::optional`, `std::expected`, error codes |
| 09 | [Move Semantics](part1-fundamentals/09-move-semantics/README.md) | lvalues/rvalues, move ctor, perfect forwarding |
| 10 | [Modern C++ Toolkit](part1-fundamentals/10-modern-cpp/README.md) | lambdas, `variant`, ranges, `constexpr`, structured bindings |
| 11 | [Concurrency](part1-fundamentals/11-concurrency/README.md) | threads, mutexes, atomics, futures, thread pools |
| 12 | [Build Systems & Testing](part1-fundamentals/12-build-and-test/README.md) | CMake, libraries, unit testing, sanitizers |

### Part 2 — Persistence
| # | Chapter | You will learn |
|---|---------|----------------|
| 13 | [Files & Serialization](part2-persistence/13-files-serialization/README.md) | streams, text/binary files, `std::filesystem`, CSV, JSON |
| 14 | [Databases with PostgreSQL](part2-persistence/14-postgresql/README.md) | SQL refresher, `libpqxx`, transactions, prepared statements, migrations |

### Part 3 — Project: REST CRUD API
| # | Chapter | You will learn |
|---|---------|----------------|
| 15 | [Tasks API](part3-crud-api/README.md) | HTTP, JSON, layered architecture, repository pattern, PostgreSQL, tests |

### Part 4 — Unreal Engine 5 Game Development
| # | Chapter | You will learn |
|---|---------|----------------|
| 16 | [Unreal C++ Primer](part4-unreal/16-unreal-primer/README.md) | UE project layout, reflection, `UObject`, GC, build tools |
| 17 | [Actors & Components](part4-unreal/17-actors-components/README.md) | `AActor`, `UActorComponent`, lifecycle, `UPROPERTY`/`UFUNCTION` |
| 18 | [Characters & Enhanced Input](part4-unreal/18-character-input/README.md) | `ACharacter`, Enhanced Input, cameras |
| 19 | [Gameplay Systems](part4-unreal/19-gameplay-systems/README.md) | health/damage, delegates, interfaces, timers, GameMode |
| 20 | [UI & Save Games](part4-unreal/20-ui-and-saving/README.md) | UMG widgets from C++, `USaveGame` persistence |

### Part 5 — C and Low-Level Programming
| # | Chapter | You will learn |
|---|---------|----------------|
| 21 | [C Essentials for C++ Programmers](part5-c-lowlevel/21-c-essentials/README.md) | what C lacks, structs & strings, the preprocessor, opaque types, `goto` cleanup |
| 22 | [Memory Management in C](part5-c-lowlevel/22-c-memory/README.md) | `malloc`/`realloc`/`free`, ownership, dynamic arrays, arena & pool allocators, your own `malloc`, ASan |
| 23 | [Writing a Hardware Controller](part5-c-lowlevel/23-hardware-controller/README.md) | memory-mapped registers, `volatile`, interrupts, drivers, a simulated fan controller, a bare-metal STM32 port |
| 24 | [Combining C and Assembly](part5-c-lowlevel/24-c-and-asm/README.md) | calling conventions, inline asm, `.S` files for x86-64 & AArch64, intrinsics |

## Getting started

Read **[docs/00-setup.md](docs/00-setup.md)** first. Short version:

```bash
# 1. Quick check without CMake: compiles (and with --run, runs) every example & solution
./scripts/check.sh            # or: ./scripts/check.sh --run 06-stl

# 2. With CMake (recommended): parts 1–2, plus tests
cmake -S . -B build
cmake --build build -j
./build/part1-fundamentals/01-basics/ch01_ex_01_hello_world
ctest --test-dir build --output-on-failure

# 3. Everything, including PostgreSQL (ch. 14) and the API (part 3), with no local installs:
docker compose up -d db
docker compose run --rm dev bash -c \
  "cmake -S . -B build-docker -G Ninja -DCLEARN_BUILD_POSTGRES=ON -DCLEARN_BUILD_API=ON && cmake --build build-docker && ctest --test-dir build-docker"

# 4. Just run the API:
cd part3-crud-api && docker compose up --build
```

## What has been verified

| Part | How it was checked |
|------|--------------------|
| 1–2 (135 example/solution programs + 5 multi-file CMake projects) | compiled with `-Wall -Wextra -Wpedantic` with Apple clang 21 (macOS) and run; the CMake super-build + `ctest` pass |
| 14 (PostgreSQL) | built with GCC 14 (Linux container), examples and solutions run against PostgreSQL 16 |
| 3 (Tasks API) | clang + GCC 14; 24 doctest cases incl. PostgreSQL contract tests; curl smoke test against the Docker stack; exercise solutions patch (30 cases) |
| 4 (Unreal) | written against the UE 5.4/5.5 APIs, but **not compiled** (needs an engine install): see the part 4 README |
| 5 (C & low level) | 42 C programs + the FC-1 simulator project, compiled with `-Wall -Wextra -Wpedantic` with Apple clang 21 on x86-64 macOS and run (chapter 22 also under ASan/UBSan); the AArch64 code is cross-compiled and linked with `-arch arm64`, and the `.S` files are also cross-assembled for Linux ELF. The STM32 port is cross-compiled but has not been run on a board |

## How to study

1. Read the chapter `README.md` top to bottom.
2. Run every program in `examples/`, then **change it** and predict the output.
3. Attempt each exercise *before* opening `solutions/`.
4. Compare your solution with ours; ours is one valid answer, not the only one.
5. Keep [docs/cheatsheet.md](docs/cheatsheet.md) and [docs/glossary.md](docs/glossary.md) open.

## Repository layout

```
cLearn/
├── CMakeLists.txt          ← builds parts 1, 2 and 5 (and the API, opt-in)
├── compose.yaml            ← PostgreSQL + a Linux dev container (docker/dev.Dockerfile)
├── cmake/                  ← shared CMake helpers (Chapter.cmake, Deps.cmake)
├── docs/                   ← setup, cheatsheet, glossary, further reading
├── scripts/check.sh        ← compile-everything without CMake
├── part1-fundamentals/
├── part2-persistence/
├── part3-crud-api/         ← standalone CMake project (also buildable from root)
├── part4-unreal/           ← UE5 source + guides (built inside an Unreal project)
└── part5-c-lowlevel/       ← C17, a simulated microcontroller, a bare-metal STM32 port, x86-64/AArch64 asm
```
