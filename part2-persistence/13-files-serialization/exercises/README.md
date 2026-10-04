# 13 — Exercises: Files & Serialization

All solutions write into `std::filesystem::temp_directory_path()` and clean up
after themselves.

### Ex 1 — Contacts book in CSV ⭐⭐
`ContactBook` with `add`, `find_by_name` (case-insensitive substring),
`remove(id)`, `save(path)` and `static load(path)`. Store the contacts as CSV
with a header, using the escaping rules from example 03. Prove that a contact
whose name contains a comma and a quote survives a save/load round trip.
→ `solutions/ex01_contacts_csv.cpp`

### Ex 2 — Binary high-score table ⭐⭐
A file holds exactly 10 fixed-size records (name[16], score), sorted
descending, with a header containing a magic number and a version.
`submit(name, score)` inserts the score if it qualifies, shifts the lower ones
down, and rewrites only the records that changed (`seekp`). Print the table
after a few submissions.
→ `solutions/ex02_highscores_binary.cpp`

### Ex 3 — Disk usage report ⭐⭐
Given a directory, walk it recursively and print: the total size, the number
of files, the bytes per extension (sorted by size), and the 5 largest files.
Skip entries you can't read (use the `error_code` overloads and
`directory_options::skip_permission_denied`). Default to scanning the repo's
`part1-fundamentals` folder.
→ `solutions/ex03_disk_usage.cpp`

### Ex 4 — Append-only key-value store ⭐⭐⭐
Build `KvStore` backed by a **log file**:
- `set(k, v)` appends `SET <len>:<k> <len>:<v>\n`; `del(k)` appends `DEL <len>:<k>\n`
  (length prefixes make any bytes safe, including spaces and newlines)
- the constructor **replays** the log to rebuild an in-memory `unordered_map`
- `compact()` rewrites the log with only the live keys (temp file + rename)
- a truncated last line (a simulated crash) must be ignored, not fatal
Show that the data survives "restarts" (constructing a new `KvStore` on the
same file) and that compaction shrinks the file.
→ `solutions/ex04_append_only_kv.cpp`

### Ex 5 — JSON task file with nlohmann/json ⭐⭐
Build a small CLI-style program (driven by a hard-coded list of commands)
that keeps a `tasks.json` file: `add <title>`, `done <id>`, `remove <id>`,
`list`. Load it at startup, save after each mutation with an atomic write, and
handle a corrupt file by starting empty and keeping a `.corrupt` copy. This
is the persistence layer of the API in miniature.
→ `solutions/ex05_json_tasks/main.cpp` (built by CMake)
