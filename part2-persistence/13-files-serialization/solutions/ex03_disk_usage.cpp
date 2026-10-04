#include <algorithm>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;

std::string human(std::uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    double v = static_cast<double>(bytes);
    int u = 0;
    while (v >= 1024 && u < 3) { v /= 1024; ++u; }
    return std::format("{:.1f} {}", v, units[u]);
}

int main(int argc, char* argv[]) {
    fs::path root = argc > 1 ? fs::path(argv[1]) : fs::path(__FILE__).parent_path() / "../../../part1-fundamentals";
    std::error_code ec;
    root = fs::canonical(root, ec);
    if (ec) {
        std::cerr << "cannot open directory: " << ec.message() << '\n';
        return 1;
    }

    std::uintmax_t total = 0;
    std::size_t files = 0;
    std::map<std::string, std::uintmax_t> by_ext;
    std::vector<std::pair<std::uintmax_t, fs::path>> sizes;

    fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
    for (; !ec && it != end; it.increment(ec)) {
        if (!it->is_regular_file(ec)) continue;
        auto sz = it->file_size(ec);
        if (ec) { ec.clear(); continue; }
        ++files;
        total += sz;
        auto ext = it->path().extension().string();
        by_ext[ext.empty() ? "(none)" : ext] += sz;
        sizes.emplace_back(sz, it->path());
    }

    std::cout << "Scanned: " << root.string() << '\n';
    std::cout << std::format("{} files, {}\n\nBy extension:\n", files, human(total));
    std::vector<std::pair<std::string, std::uintmax_t>> ext_sorted(by_ext.begin(), by_ext.end());
    std::ranges::sort(ext_sorted, std::ranges::greater{}, &std::pair<std::string, std::uintmax_t>::second);
    for (const auto& [ext, b] : ext_sorted) std::cout << std::format("  {:<10} {:>10}\n", ext, human(b));

    std::cout << "\nLargest files:\n";
    auto n = std::min<std::size_t>(5, sizes.size());
    std::partial_sort(sizes.begin(), sizes.begin() + static_cast<std::ptrdiff_t>(n), sizes.end(), std::greater<>{});
    for (std::size_t i = 0; i < n; ++i)
        std::cout << std::format("  {:>10}  {}\n", human(sizes[i].first), fs::relative(sizes[i].second, root).string());
}
