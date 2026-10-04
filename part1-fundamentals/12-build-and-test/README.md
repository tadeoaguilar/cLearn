# 12 — Build Systems, Libraries & Testing

> Goal: structure real projects. That means splitting code into libraries,
> building with CMake, managing dependencies, writing unit tests, and using
> sanitizers and debuggers. Everything in parts 3 and 4 depends on this.

## 1. What a build system does

For one file, `clang++ main.cpp` is enough. Real projects have hundreds of
files, external libraries, and debug/release configurations. A build system:
- tracks dependencies and recompiles **only what changed**,
- passes the right flags, include paths and libraries,
- works across compilers and platforms.

**CMake** doesn't build anything itself. It *generates* build files for
Ninja, Make, Xcode or Visual Studio. In CMake terms:

```
CMakeLists.txt ──cmake -S . -B build──► build/ (Ninja/Make files) ──cmake --build build──► binaries
```

## 2. Modern CMake: think in **targets**

```cmake
cmake_minimum_required(VERSION 3.20)
project(TextStats LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# A library target
add_library(textstats src/textstats.cpp)
target_include_directories(textstats PUBLIC include)   # users of textstats get this include path
target_compile_features(textstats PUBLIC cxx_std_23)

# An executable that uses it
add_executable(textstats_cli app/main.cpp)
target_link_libraries(textstats_cli PRIVATE textstats)  # inherits textstats' PUBLIC settings
```

### PUBLIC / PRIVATE / INTERFACE
| Keyword | Applies to this target | Propagates to dependents |
|---------|------------------------|--------------------------|
| `PRIVATE` | ✅ | ❌ |
| `PUBLIC` | ✅ | ✅ |
| `INTERFACE` | ❌ | ✅ (header-only libraries) |

Rule: if the dependency appears in your **headers**, make it `PUBLIC`. If it's
only used in `.cpp` files, make it `PRIVATE`.

### Avoid old-style CMake
❌ `include_directories()`, `link_libraries()`, `set(CMAKE_CXX_FLAGS ...)`
apply globally and leak everywhere.
✅ `target_*` commands scope settings to a target.

## 3. Project layout

```
project/
├── CMakeLists.txt
├── include/textstats/textstats.hpp   ← public API (namespaced folder avoids header name clashes)
├── src/textstats.cpp                 ← implementation
├── app/main.cpp                      ← a CLI that uses the library
└── tests/test_textstats.cpp          ← unit tests
```
Keeping logic in a **library** and keeping `main` thin makes the logic
testable. The CRUD API follows the same layout.

## 4. Static vs shared libraries

| | Static (`.a`, `.lib`) | Shared (`.so`, `.dylib`, `.dll`) |
|-|-----------------------|----------------------------------|
| Linked | into the executable at build time | loaded at runtime |
| Deploy | single binary | ship the library too |
| Update | rebuild the app | swap the library (ABI permitting) |
`add_library(name STATIC ...)` or `SHARED`. If you don't specify,
`BUILD_SHARED_LIBS` decides.

Unreal Engine modules are shared libraries in the editor, and statically
linked in shipped games.

## 5. Dependencies

| Method | When |
|--------|------|
| `find_package(X REQUIRED)` | library is installed on the system (Homebrew, apt, vcpkg) |
| `FetchContent` | download and build the source at configure time (great for small libs) |
| **vcpkg** / **Conan** | package managers for bigger dependency graphs |
| `pkg_check_modules` | libs that ship `.pc` files (e.g. libpqxx) |

```cmake
include(FetchContent)
FetchContent_Declare(doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.12)
FetchContent_MakeAvailable(doctest)
target_link_libraries(my_tests PRIVATE doctest::doctest)
```

## 6. Build types

```bash
cmake -S . -B build-debug   -DCMAKE_BUILD_TYPE=Debug          # -g, no optimization
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release        # -O3 -DNDEBUG
cmake -S . -B build-asan    -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
```
Use one build directory per configuration. Never build inside the source tree.

## 7. Unit testing

A unit test calls a small piece of code with known inputs and **asserts**
on the outputs. Tests:
- catch regressions when you refactor,
- document how the code is supposed to be used,
- push you towards a better design (code that is hard to test is usually too coupled).

We use **doctest**: a single header, fast to compile, with Catch2-like syntax.
**GoogleTest** and **Catch2** are equally popular.

```cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_CASE("word_count counts words separated by whitespace") {
    CHECK(ts::word_count("") == 0);
    CHECK(ts::word_count("one two  three") == 3);

    SUBCASE("punctuation is not a word") {
        CHECK(ts::word_count(" , ") == 0);
    }
}
TEST_CASE("throws on invalid input") {
    CHECK_THROWS_AS(ts::top_words("x", 0), std::invalid_argument);
}
```
`CHECK` keeps going after a failure. `REQUIRE` stops the test case.

CMake integration: `enable_testing()` + `add_test(NAME ... COMMAND ...)`, then
run them with **`ctest --test-dir build --output-on-failure`**.

### Arrange–Act–Assert
```cpp
TEST_CASE("deposit increases balance") {
    BankAccount acc{"ada", 100};   // Arrange
    acc.deposit(50);               // Act
    CHECK(acc.balance() == 150);   // Assert
}
```

### Test doubles
To test code that talks to a database or the network, put it behind an
**interface**, and give the tests a fake in-memory implementation. Chapter 4
showed `MemoryLogger`. The CRUD API tests use `InMemoryTaskRepository`.

## 8. Sanitizers & debugging

| Tool | Flag | Finds |
|------|------|-------|
| AddressSanitizer | `-fsanitize=address` | out-of-bounds, use-after-free, leaks (Linux) |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` | signed overflow, bad shifts, null deref... |
| ThreadSanitizer | `-fsanitize=thread` | data races (can't combine with ASan) |
| lldb / gdb | `-g` | step through code: `lldb ./app`, `b main`, `r`, `n`, `s`, `p var`, `bt` |

Other tools: **clang-tidy** (static analysis, modernization hints),
**clang-format** (consistent style; this repo has a `.clang-format`),
and **compiler warnings** (`-Wall -Wextra -Wpedantic -Werror` in CI).

## 9. The preprocessor

```cpp
#include <file>      // paste a file
#define DEBUG 1      // text substitution: no types, no scopes. Prefer constexpr
#ifdef _WIN32 ... #endif   // conditional compilation (platform-specific code)
#pragma once         // include guard
__FILE__, __LINE__   // prefer std::source_location
```
Macros are still common in **Unreal Engine** (`UCLASS()`, `UPROPERTY()`,
`UE_LOG`, `check()`), so it's worth understanding what they do. In your own
code, prefer `constexpr`, `inline` functions and templates.

## Build this chapter

```bash
# From the repo root (builds all chapters, including this one):
cmake -S . -B build && cmake --build build
ctest --test-dir build --output-on-failure

# Or just this mini project:
cmake -S part1-fundamentals/12-build-and-test -B build-ch12 && cmake --build build-ch12
./build-ch12/project/textstats_cli part1-fundamentals/12-build-and-test/README.md
```

## Files
| Path | Shows |
|------|-------|
| `project/` | library + CLI + doctest tests, with modern CMake |
| `examples/01_preprocessor.cpp` | macros, conditional compilation, why to prefer constexpr |
| `examples/02_mini_test_framework.cpp` | how a test framework works inside (~60 lines) |
| `examples/03_sanitizer_targets.cpp` | bugs that ASan/UBSan catch (enable by argument) |
