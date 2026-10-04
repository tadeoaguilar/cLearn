// Crash-safe saving: write a temp file, then atomically rename it over the original.
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

void atomic_write(const fs::path& target, const std::string& contents, bool keep_backup = true) {
    fs::path tmp = target;
    tmp += ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) throw std::runtime_error("cannot open temp file");
        out << contents;
        out.flush();
        if (!out) throw std::runtime_error("write failed"); // e.g. disk full: original untouched
        // For real durability you would also fsync() the file and its directory (POSIX API).
    }
    if (keep_backup && fs::exists(target)) fs::copy_file(target, fs::path(target) += ".bak", fs::copy_options::overwrite_existing);
    fs::rename(tmp, target); // atomic replace on POSIX (same filesystem)
}

std::string read_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

int main() {
    const fs::path dir = fs::temp_directory_path() / "clearn_atomic";
    fs::create_directories(dir);
    const fs::path save = dir / "savegame.txt";

    atomic_write(save, "level=1\nhp=100\n");
    atomic_write(save, "level=2\nhp=87\n");

    std::cout << "current:\n" << read_file(save);
    std::cout << "backup:\n" << read_file(fs::path(save) += ".bak");
    std::cout << "temp left behind? " << std::boolalpha << fs::exists(fs::path(save) += ".tmp") << '\n';

    // Simulated crash: a half-written temp file never replaces the real one
    std::ofstream(fs::path(save) += ".tmp") << "level=3\nh"; // "crash" here, no rename
    std::cout << "after crash, save is still valid:\n" << read_file(save);

    fs::remove_all(dir);
}
