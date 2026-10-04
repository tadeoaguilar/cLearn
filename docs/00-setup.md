# 00 — Environment Setup (macOS first, Linux/Windows notes)

## 1. Compiler

C++ is a **compiled** language. Source files (`.cpp`) go through a compiler
(clang, gcc or MSVC). The compiler produces object files, and a linker joins
them into an executable.

On macOS, Apple's clang comes with the Xcode Command Line Tools:

```bash
xcode-select --install     # only if `clang++ --version` fails
clang++ --version          # need clang 17+ (Apple clang 16+) for the C++23 bits we use
```

Try it right away:

```bash
clang++ -std=c++23 -Wall -Wextra part1-fundamentals/01-basics/examples/01_hello_world.cpp -o hello
./hello
```

| Flag | Meaning |
|------|---------|
| `-std=c++23` | use the C++23 language standard (the repo's default; most code is C++20) |
| `-Wall -Wextra -Wpedantic` | turn on most warnings (treat them like errors when learning!) |
| `-g` | include debug info (for lldb / gdb) |
| `-O2` | optimize |
| `-fsanitize=address,undefined` | catch memory bugs & UB at runtime (see ch. 12) |
| `-o name` | output file name |

## 2. Homebrew and CMake

[CMake](https://cmake.org) is the de-facto build system generator for C++.
Install Homebrew (https://brew.sh), then:

```bash
brew install cmake ninja pkg-config
```

If you prefer no Homebrew, download the CMake `.dmg` from cmake.org and run
`sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install`.

Build everything:

```bash
cmake -S . -B build -G Ninja          # configure (once)
cmake --build build                   # compile (every time you change code)
ls build/part1-fundamentals/01-basics # the executables
```

## 3. PostgreSQL (parts 2 & 3)

The easiest path is **Docker**: install Docker Desktop or [OrbStack](https://orbstack.dev).

```bash
docker compose up -d db               # from the repo root: Postgres 16 on localhost:5432
                                      # (part3-crud-api has its own stack on :5433 + the API on :8080)
```

The C++ client library **libpqxx** is downloaded and built by our CMake (a
pinned version, so everyone uses the same API). It sits on top of PostgreSQL's
C library **libpq**, which you install:

```bash
brew install libpq            # "keg-only": tell CMake where it is ↓
cmake -S . -B build -DCLEARN_BUILD_POSTGRES=ON -DCMAKE_PREFIX_PATH="$(brew --prefix libpq)"
echo 'export PATH="$(brew --prefix libpq)/bin:$PATH"' >> ~/.zshrc   # optional: puts psql on your PATH
```

Linux (Debian/Ubuntu): `sudo apt install libpq-dev postgresql-client`.

### No local installs: the dev container

`compose.yaml` at the repo root also defines a `dev` service: Ubuntu 24.04 with
GCC 14, CMake, Ninja and libpq. Your checkout is mounted at `/workspace`, and
`DATABASE_URL` points at the `db` service:

```bash
docker compose up -d db
docker compose run --rm dev            # interactive shell
# inside:
cmake -S . -B build-docker -G Ninja -DCLEARN_BUILD_POSTGRES=ON -DCLEARN_BUILD_API=ON
cmake --build build-docker
./build-docker/part2-persistence/14-postgresql/ch14_ex_01_connect
```
Building with a second compiler (GCC here, clang on your Mac) catches portability
bugs. Part 3's docs tell the story of one we caught this way.

## 4. Editor

- **VS Code** with the *clangd* extension (best completion; uses
  `build/compile_commands.json` that our CMake generates) and *CodeLLDB* for
  debugging.
- **CLion**: opens the CMake project directly.
- **Xcode** or **Rider** for Unreal Engine on macOS; **Visual Studio 2022** or **Rider** on Windows.

Link `compile_commands.json` into the root so clangd finds it:

```bash
ln -s build/compile_commands.json .
```

## 5. Unreal Engine (part 4)

1. Install the **Epic Games Launcher**, then **Unreal Engine 5.4+** from the Library tab.
2. macOS: install the **full Xcode** from the App Store (not just the command
   line tools). Windows: Visual Studio 2022 with "Game development with C++".
3. Details are in [chapter 16](../part4-unreal/16-unreal-primer/README.md).

## 6. Sanity check

```bash
./scripts/check.sh          # every example & solution should say OK
./scripts/check.sh --run    # also run them
```
