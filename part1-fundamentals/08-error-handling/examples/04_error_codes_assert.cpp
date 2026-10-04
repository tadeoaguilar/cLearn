#include <cassert>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

static_assert(sizeof(long long) >= 8, "this program needs 64-bit long long");

int element_at(const std::vector<int>& v, std::size_t i) {
    assert(i < v.size() && "element_at: index out of range"); // checks OUR logic in debug builds
    return v[i];
}

int main() {
    // Non-throwing filesystem overload: report via error_code
    std::error_code ec;
    auto size = fs::file_size("definitely_missing_file.txt", ec);
    if (ec) {
        std::cout << "file_size failed: " << ec.message() << " (value " << ec.value() << ")\n";
    } else {
        std::cout << "size " << size << '\n';
    }

    // Throwing overload of the same function
    try {
        fs::file_size("definitely_missing_file.txt");
    } catch (const fs::filesystem_error& e) {
        std::cout << "threw filesystem_error: " << e.code().message() << '\n';
    }

    // Comparing against portable error conditions
    if (ec == std::errc::no_such_file_or_directory) std::cout << "it's a 'no such file' error\n";

    std::vector<int> v{1, 2, 3};
    std::cout << "element_at(v, 1) = " << element_at(v, 1) << '\n';
    // element_at(v, 5); // debug build: assertion failure aborts with file:line
    std::cout << "(build with -DNDEBUG and assert() disappears)\n";
}
