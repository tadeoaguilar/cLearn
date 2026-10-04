#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

constexpr std::uint32_t kMagic = 0x48495343; // "HISC"
constexpr std::uint32_t kVersion = 1;
constexpr std::size_t kSlots = 10;

struct Header { std::uint32_t magic = kMagic, version = kVersion; };
struct Entry { std::array<char, 16> name{}; std::uint32_t score = 0; };
static_assert(sizeof(Entry) == 20);

class HighScores {
public:
    explicit HighScores(fs::path p) : path_{std::move(p)} {
        if (!fs::exists(path_)) create();
        f_.open(path_, std::ios::in | std::ios::out | std::ios::binary);
        Header h;
        f_.read(reinterpret_cast<char*>(&h), sizeof h);
        if (!f_ || h.magic != kMagic || h.version != kVersion) throw std::runtime_error("bad high-score file");
        for (auto& e : table_) f_.read(reinterpret_cast<char*>(&e), sizeof e);
    }

    // Returns the rank (0-based) or -1 if the score didn't qualify.
    int submit(const std::string& name, std::uint32_t score) {
        std::size_t pos = 0;
        while (pos < kSlots && table_[pos].score >= score) ++pos;
        if (pos == kSlots) return -1;
        for (std::size_t i = kSlots - 1; i > pos; --i) table_[i] = table_[i - 1]; // shift down
        Entry e;
        std::strncpy(e.name.data(), name.c_str(), e.name.size() - 1);
        e.score = score;
        table_[pos] = e;
        // Only records pos..end changed: rewrite just those
        f_.seekp(static_cast<std::streamoff>(sizeof(Header) + pos * sizeof(Entry)));
        for (std::size_t i = pos; i < kSlots; ++i) f_.write(reinterpret_cast<const char*>(&table_[i]), sizeof(Entry));
        f_.flush();
        return static_cast<int>(pos);
    }

    void print() const {
        for (std::size_t i = 0; i < kSlots; ++i)
            if (table_[i].score) std::cout << "  " << i + 1 << ". " << table_[i].name.data() << " " << table_[i].score << '\n';
    }

private:
    void create() {
        std::ofstream out(path_, std::ios::binary);
        Header h;
        out.write(reinterpret_cast<const char*>(&h), sizeof h);
        Entry empty;
        for (std::size_t i = 0; i < kSlots; ++i) out.write(reinterpret_cast<const char*>(&empty), sizeof empty);
    }

    fs::path path_;
    std::fstream f_;
    std::array<Entry, kSlots> table_{};
};

int main() {
    const fs::path file = fs::temp_directory_path() / "clearn_highscores.bin";
    fs::remove(file);
    {
        HighScores hs{file};
        for (auto [name, score] : {std::pair{"ada", 500u}, {"bob", 300u}, {"cy", 900u}, {"dee", 300u}, {"eve", 50u}})
            std::cout << name << " " << score << " -> rank " << hs.submit(name, score) + 1 << '\n';
    }
    HighScores reopened{file}; // persisted!
    std::cout << "after reopening:\n";
    reopened.print();
    std::cout << "file size: " << fs::file_size(file) << " bytes (always 8 + 10*20)\n";
    fs::remove(file);
}
