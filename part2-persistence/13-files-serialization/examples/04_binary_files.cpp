// Fixed-size binary records with a versioned header and random access.
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace fs = std::filesystem;

struct FileHeader {
    std::array<char, 4> magic{'S', 'C', 'O', 'R'}; // identifies our format
    std::uint32_t version = 1;
    std::uint32_t count = 0;
};

struct ScoreRecord {
    std::array<char, 16> name{}; // fixed-size, no std::string (it holds a pointer!)
    std::uint32_t score = 0;
    std::uint32_t level = 0;
};

static_assert(std::is_trivially_copyable_v<FileHeader> && std::is_trivially_copyable_v<ScoreRecord>);
static_assert(sizeof(ScoreRecord) == 24, "unexpected padding: format would change");
static_assert(std::endian::native == std::endian::little, "this demo assumes a little-endian machine");

ScoreRecord make(const std::string& name, std::uint32_t score, std::uint32_t level) {
    ScoreRecord r;
    std::strncpy(r.name.data(), name.c_str(), r.name.size() - 1); // always leaves a terminating '\0'
    r.score = score;
    r.level = level;
    return r;
}

template <typename T>
void write_pod(std::ostream& out, const T& v) { out.write(reinterpret_cast<const char*>(&v), sizeof v); }
template <typename T>
void read_pod(std::istream& in, T& v) {
    if (!in.read(reinterpret_cast<char*>(&v), sizeof v)) throw std::runtime_error("truncated file");
}

int main() {
    const fs::path file = fs::temp_directory_path() / "clearn_scores.bin";
    const ScoreRecord records[] = {make("ada", 9100, 7), make("linus", 8700, 6), make("grace", 9900, 9)};

    {
        std::ofstream out(file, std::ios::binary);
        FileHeader h;
        h.count = std::size(records);
        write_pod(out, h);
        for (const auto& r : records) write_pod(out, r);
    }
    std::cout << "file size: " << fs::file_size(file) << " bytes = " << sizeof(FileHeader) << " header + 3 x "
              << sizeof(ScoreRecord) << '\n';

    std::fstream f(file, std::ios::in | std::ios::out | std::ios::binary);
    FileHeader h;
    read_pod(f, h);
    if (std::string(h.magic.data(), 4) != "SCOR") throw std::runtime_error("not a score file");
    if (h.version != 1) throw std::runtime_error("unsupported version");

    // Random access: jump straight to record #2 without reading #0 and #1
    auto offset_of = [](std::uint32_t i) { return static_cast<std::streamoff>(sizeof(FileHeader) + i * sizeof(ScoreRecord)); };
    ScoreRecord r;
    f.seekg(offset_of(2));
    read_pod(f, r);
    std::cout << "record 2: " << r.name.data() << " " << r.score << '\n';

    // Update record #1 in place
    r = make("linus", 9500, 7);
    f.seekp(offset_of(1));
    write_pod(f, r);
    f.flush();

    f.seekg(offset_of(0));
    for (std::uint32_t i = 0; i < h.count; ++i) {
        read_pod(f, r);
        std::cout << i << ": " << r.name.data() << " score=" << r.score << " level=" << r.level << '\n';
    }
    f.close();
    fs::remove(file);
}
