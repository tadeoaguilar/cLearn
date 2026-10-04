#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

namespace fs = std::filesystem;

int main() {
    const fs::path root = fs::temp_directory_path() / "clearn_fs_demo";
    fs::remove_all(root); // start clean (no error if missing)

    // Build a small tree
    fs::create_directories(root / "saves" / "slot1");
    fs::create_directories(root / "logs");
    std::ofstream(root / "saves" / "slot1" / "game.sav") << "level=3\nhp=42\n";
    std::ofstream(root / "saves" / "settings.json") << R"({"volume": 0.8})";
    std::ofstream(root / "logs" / "app.log") << "started\nloaded save\n";

    // Path decomposition
    fs::path p = root / "saves" / "slot1" / "game.sav";
    std::cout << "filename:  " << p.filename() << '\n';
    std::cout << "stem:      " << p.stem() << '\n';
    std::cout << "extension: " << p.extension() << '\n';
    std::cout << "parent:    " << p.parent_path().filename() << '\n';
    std::cout << "relative:  " << fs::relative(p, root) << "\n\n";

    // Recursive walk with metadata
    std::map<std::string, std::uintmax_t> bytes_by_ext;
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        auto rel = fs::relative(entry.path(), root);
        std::cout << std::string(2 * static_cast<std::size_t>(std::distance(rel.begin(), rel.end()) - 1), ' ')
                  << (entry.is_directory() ? "[d] " : "    ") << entry.path().filename().string();
        if (entry.is_regular_file()) {
            std::cout << " (" << entry.file_size() << " B)";
            bytes_by_ext[entry.path().extension().string()] += entry.file_size();
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    for (const auto& [ext, b] : bytes_by_ext) std::cout << ext << ": " << b << " bytes\n";

    // Copy, rename, remove
    fs::copy_file(p, root / "saves" / "slot1" / "game.bak", fs::copy_options::overwrite_existing);
    fs::rename(root / "logs" / "app.log", root / "logs" / "app.old.log");
    std::cout << "\nbackup exists? " << std::boolalpha << fs::exists(root / "saves" / "slot1" / "game.bak") << '\n';

    // Non-throwing variant
    std::error_code ec;
    fs::remove(root / "nope" / "missing.txt", ec);
    std::cout << "remove missing: " << (ec ? ec.message() : "no error (nothing to remove)") << '\n';

    auto removed = fs::remove_all(root);
    std::cout << "cleaned up " << removed << " entries\n";
}
