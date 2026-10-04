// A correct CSV writer/reader (RFC 4180): quoted fields, escaped quotes, embedded commas/newlines.
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::string csv_escape(const std::string& field) {
    if (field.find_first_of(",\"\n\r") == std::string::npos) return field;
    std::string out = "\"";
    for (char c : field) {
        if (c == '"') out += "\"\""; // double the quote
        else out += c;
    }
    return out + "\"";
}

void write_row(std::ostream& out, const std::vector<std::string>& fields) {
    for (std::size_t i = 0; i < fields.size(); ++i) out << (i ? "," : "") << csv_escape(fields[i]);
    out << '\n';
}

// Reads one logical record (which may span several physical lines if a quoted field has newlines)
std::optional<std::vector<std::string>> read_row(std::istream& in) {
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false, any = false;
    char c;
    while (in.get(c)) {
        any = true;
        if (in_quotes) {
            if (c == '"') {
                if (in.peek() == '"') { field += '"'; in.get(); } // escaped quote
                else in_quotes = false;
            } else {
                field += c;
            }
        } else if (c == '"') {
            in_quotes = true;
        } else if (c == ',') {
            fields.push_back(std::move(field));
            field.clear();
        } else if (c == '\n') {
            break;
        } else if (c != '\r') {
            field += c;
        }
    }
    if (!any) return std::nullopt;
    fields.push_back(std::move(field));
    return fields;
}

struct Product {
    int id;
    std::string name;
    double price;
    std::string notes;
};

int main() {
    const fs::path file = fs::temp_directory_path() / "clearn_products.csv";
    std::vector<Product> products{
        {1, "Coffee, dark roast", 12.5, "best seller"},
        {2, "Mug \"Hello\"", 8.0, "ceramic"},
        {3, "Gift card", 25.0, "line one\nline two"},
    };

    {
        std::ofstream out(file);
        write_row(out, {"id", "name", "price", "notes"});
        for (const auto& p : products) write_row(out, {std::to_string(p.id), p.name, std::to_string(p.price), p.notes});
    }

    std::cout << "--- raw file ---\n";
    std::cout << std::ifstream(file).rdbuf() << '\n';

    std::cout << "--- parsed back ---\n";
    std::ifstream in(file);
    auto header = read_row(in); // skip header
    std::vector<Product> loaded;
    while (auto row = read_row(in)) {
        if (row->size() != 4) {
            std::cerr << "skipping malformed row\n";
            continue;
        }
        loaded.push_back({std::stoi((*row)[0]), (*row)[1], std::stod((*row)[2]), (*row)[3]});
    }
    for (const auto& p : loaded) std::cout << p.id << " | " << p.name << " | " << p.price << " | [" << p.notes << "]\n";
    std::cout << "round trip ok? " << std::boolalpha
              << (loaded.size() == products.size() && loaded[1].name == products[1].name && loaded[2].notes == products[2].notes) << '\n';
    fs::remove(file);
}
