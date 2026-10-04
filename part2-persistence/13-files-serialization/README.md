# 13 — Files, Filesystem & Serialization

> Goal: make data **outlive the process**. You'll read and write text and
> binary files, manage directories, design file formats, avoid corrupting data
> on crashes, and serialize objects to CSV and JSON. These are the
> foundations under every database.

## 1. Persistence: the big picture

| Approach | Pros | Cons | Use for |
|----------|------|------|---------|
| Text files (custom, CSV) | human-readable, simple, diffable | parsing work, slow for big data | configs, exports, logs |
| JSON / YAML / TOML | self-describing, universal | verbose, no schema by default | configs, API payloads, small data |
| Binary files | compact, fast, random access | not human-readable, versioning & endianness | game saves, caches, assets |
| Embedded DB (SQLite) | SQL, transactions, single file | one writer at a time | apps, mobile, tools |
| Server DB (**PostgreSQL**) | concurrency, durability, scale, constraints | needs a server | **web APIs** (ch. 14–15) |

Every option faces the same questions: **format** (how bytes represent your
objects), **durability** (what happens on a crash mid-write), **evolution**
(how version 2 reads version 1 data) and **concurrency** (two writers at once).

## 2. Streams recap

```
               std::ios_base
                    │
              std::basic_ios
           ┌────────┴─────────┐
      std::istream        std::ostream
       │       └──std::iostream──┘      │
  std::ifstream    std::fstream    std::ofstream   (files)
  std::istringstream               std::ostringstream (strings)
```
The same `>>` / `<<` / `getline` code works for the console, files and strings.

## 3. Text files

```cpp
#include <fstream>

std::ofstream out("notes.txt");            // truncates (creates if missing)
std::ofstream log("app.log", std::ios::app); // append mode
if (!out) throw std::runtime_error("cannot open");
out << "line 1\n";                          // closes automatically (RAII)

std::ifstream in("notes.txt");
for (std::string line; std::getline(in, line);) { ... }   // line by line
in >> word;                                               // whitespace-separated tokens
```

**Check stream state.** `if (!in)` / `in.fail()` / `in.eof()` / `in.bad()`.
A stream in a failed state ignores further operations until you call `clear()`.
Opt in to exceptions with `in.exceptions(std::ios::badbit)`.

Open modes: `std::ios::in`, `out`, `app` (always write at end), `trunc`,
`binary` (no newline translation on Windows), `ate` (start at end).

## 4. `std::filesystem`

```cpp
#include <filesystem>
namespace fs = std::filesystem;

fs::path p = fs::path("data") / "users" / "ada.json";   // portable separators
p.filename(); p.stem(); p.extension(); p.parent_path();
fs::exists(p); fs::is_directory(p); fs::file_size(p);
fs::create_directories("data/users");
fs::copy_file(a, b, fs::copy_options::overwrite_existing);
fs::rename(tmp, final);                 // atomic on POSIX within one filesystem
fs::remove(p); fs::remove_all("data");
for (const auto& e : fs::recursive_directory_iterator("data")) { e.path(); e.file_size(); }
fs::temp_directory_path();
```
Most functions have a throwing version and a `std::error_code&` version.

## 5. CSV

It looks trivial until a field contains a comma, a quote or a newline. The
rules (RFC 4180): fields containing `,` `"` or newlines are wrapped in quotes,
and quotes inside are doubled (`""`). Example 03 implements a correct
reader and writer.

## 6. Binary files

```cpp
std::ofstream out("scores.bin", std::ios::binary);
out.write(reinterpret_cast<const char*>(&record), sizeof record);  // raw bytes
in.read(reinterpret_cast<char*>(&record), sizeof record);
in.seekg(index * sizeof(Record));   // random access: jump to record N
```
Pitfalls:
- **Only trivially-copyable types** (no `std::string`, pointers or virtuals)
  can be dumped as raw bytes. Write strings as a length followed by the bytes.
- **Padding** between members: use fixed-width types and `static_assert(sizeof(...))`.
- **Endianness**: x86 and ARM are little-endian, but some formats are
  big-endian. `std::endian` and `std::byteswap` (C++23) help.
- **Versioning**: start the file with a *magic number* and a *version* field.

## 7. Serialization design

Serialization turns an object graph into bytes, and deserialization turns
them back. Good designs:
- **separate the model from the format**: `to_json(User)` and `from_json`
  live next to the model, or in a serializer, not scattered around,
- **validate on read**. Files are input, so never trust them,
- **version** your formats, and keep reading old versions,
- are **round-trip tested**: `deserialize(serialize(x)) == x`.

### JSON with nlohmann/json
The most popular C++ JSON library (header-only). It's used by the CRUD API
in part 3.
```cpp
#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct Task { int id; std::string title; bool done; };
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Task, id, title, done)   // generates to_json/from_json

json j = Task{1, "learn C++", false};      // → {"id":1,"title":"learn C++","done":false}
std::string text = j.dump(2);              // pretty print
Task t = json::parse(text).get<Task>();    // throws json::exception on bad input
```
See `json_example/` (built by CMake, which downloads the library).

## 8. Durability: don't lose data on a crash

Writing directly over `data.json` and crashing halfway leaves a **corrupt
file**. The standard safe-save pattern:

```
1. write everything to data.json.tmp
2. flush + close (optionally fsync for real durability)
3. rename data.json.tmp → data.json     ← atomic: readers see old OR new, never half
```

Another approach is the **append-only log** (write-ahead log, WAL). You never
modify old bytes, you only append operations such as `SET k v` or `DEL k`. On
startup you replay the log, and occasionally you *compact* it. Databases,
Kafka and Redis's AOF all work this way. Exercise 4 builds one.

## 9. Towards databases

Files get hard when you need: **concurrent writers**, **queries** ("all tasks
due this week, sorted by priority"), **transactions** (update two things
atomically), **constraints** (unique emails), or **data bigger than RAM**.
That's what databases do. Next chapter: PostgreSQL.

## Files
| File | Shows |
|------|-------|
| `examples/01_text_files.cpp` | write/append/read lines & tokens, stream states |
| `examples/02_filesystem.cpp` | paths, directories, iteration, metadata, cleanup |
| `examples/03_csv.cpp` | RFC-4180 CSV writer & parser into structs |
| `examples/04_binary_files.cpp` | binary records with header, random access with seek |
| `examples/05_text_serialization.cpp` | `to_string`/`from_string` for a model, round-trip test |
| `examples/06_atomic_save.cpp` | temp-file + rename, backups |
| `json_example/` | nlohmann/json: structs ↔ JSON, files, error handling (CMake) |
