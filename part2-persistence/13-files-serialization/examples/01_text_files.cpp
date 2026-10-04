#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void write_file(const fs::path& p, const std::vector<std::string>& lines) {
    std::ofstream out(p); // default mode: out | trunc
    if (!out) throw std::runtime_error("cannot open " + p.string() + " for writing");
    for (const auto& l : lines) out << l << '\n';
} // closed (and flushed) here

void append_line(const fs::path& p, const std::string& line) {
    std::ofstream out(p, std::ios::app);
    if (!out) throw std::runtime_error("cannot append to " + p.string());
    out << line << '\n';
}

std::vector<std::string> read_lines(const fs::path& p) {
    std::ifstream in(p);
    if (!in) throw std::runtime_error("cannot open " + p.string());
    std::vector<std::string> lines;
    for (std::string line; std::getline(in, line);) lines.push_back(line);
    if (in.bad()) throw std::runtime_error("I/O error while reading"); // eof is expected, bad is not
    return lines;
}

std::string read_all(const fs::path& p) {
    std::ifstream in(p);
    std::ostringstream ss;
    ss << in.rdbuf(); // slurp the whole file
    return ss.str();
}

int main() {
    const fs::path file = fs::temp_directory_path() / "clearn_text_demo.txt";

    write_file(file, {"apples 3 0.50", "bread 1 2.25", "milk 2 1.10"});
    append_line(file, "eggs 12 0.20");

    std::cout << "--- lines ---\n";
    for (const auto& l : read_lines(file)) std::cout << l << '\n';

    std::cout << "--- parsed tokens ---\n";
    std::ifstream in(file);
    std::string name;
    int qty{};
    double price{};
    double total = 0;
    while (in >> name >> qty >> price) { // stops at EOF or bad format
        total += qty * price;
        std::cout << name << ": " << qty << " x " << price << '\n';
    }
    std::cout << "total = " << total << '\n';

    std::cout << "--- whole file has " << read_all(file).size() << " bytes ---\n";

    // Stream states
    std::istringstream bad_input("42 abc");
    int a{}, b{};
    bad_input >> a >> b;
    std::cout << "a=" << a << " fail=" << bad_input.fail() << " eof=" << bad_input.eof() << '\n';
    bad_input.clear(); // reset the error flags so the stream works again
    std::string rest;
    bad_input >> rest;
    std::cout << "after clear(), rest='" << rest << "'\n";

    try {
        read_lines("/definitely/not/here.txt");
    } catch (const std::exception& e) {
        std::cout << "error: " << e.what() << '\n';
    }
    fs::remove(file);
}
