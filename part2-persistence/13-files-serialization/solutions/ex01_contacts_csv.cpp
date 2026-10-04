#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// --- CSV helpers (from example 03) ---
std::string csv_escape(const std::string& f) {
    if (f.find_first_of(",\"\n\r") == std::string::npos) return f;
    std::string out = "\"";
    for (char c : f) out += (c == '"') ? std::string("\"\"") : std::string(1, c);
    return out + "\"";
}
std::optional<std::vector<std::string>> read_row(std::istream& in) {
    std::vector<std::string> fields;
    std::string field;
    bool q = false, any = false;
    char c;
    while (in.get(c)) {
        any = true;
        if (q) {
            if (c == '"') { if (in.peek() == '"') { field += '"'; in.get(); } else q = false; }
            else field += c;
        } else if (c == '"') q = true;
        else if (c == ',') { fields.push_back(std::move(field)); field.clear(); }
        else if (c == '\n') break;
        else if (c != '\r') field += c;
    }
    if (!any) return std::nullopt;
    fields.push_back(std::move(field));
    return fields;
}

struct Contact {
    int id;
    std::string name;
    std::string email;
    std::string phone;
};

class ContactBook {
public:
    Contact& add(std::string name, std::string email, std::string phone) {
        contacts_.push_back({next_id_++, std::move(name), std::move(email), std::move(phone)});
        return contacts_.back();
    }

    std::vector<Contact> find_by_name(const std::string& needle) const {
        auto lower = [](std::string s) {
            for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return s;
        };
        std::vector<Contact> out;
        std::ranges::copy_if(contacts_, std::back_inserter(out),
                             [&](const Contact& c) { return lower(c.name).find(lower(needle)) != std::string::npos; });
        return out;
    }

    bool remove(int id) { return std::erase_if(contacts_, [id](const Contact& c) { return c.id == id; }) > 0; }

    void save(const fs::path& p) const {
        std::ofstream out(p);
        if (!out) throw std::runtime_error("cannot write " + p.string());
        out << "id,name,email,phone\n";
        for (const auto& c : contacts_)
            out << c.id << ',' << csv_escape(c.name) << ',' << csv_escape(c.email) << ',' << csv_escape(c.phone) << '\n';
    }

    static ContactBook load(const fs::path& p) {
        std::ifstream in(p);
        if (!in) throw std::runtime_error("cannot read " + p.string());
        ContactBook book;
        read_row(in); // header
        while (auto row = read_row(in)) {
            if (row->size() != 4) continue;
            int id = std::stoi((*row)[0]);
            book.contacts_.push_back({id, (*row)[1], (*row)[2], (*row)[3]});
            book.next_id_ = std::max(book.next_id_, id + 1);
        }
        return book;
    }

    std::size_t size() const { return contacts_.size(); }

private:
    std::vector<Contact> contacts_;
    int next_id_ = 1;
};

int main() {
    const fs::path file = fs::temp_directory_path() / "clearn_contacts.csv";
    {
        ContactBook book;
        book.add("Lovelace, Ada", "ada@example.com", "+44 1");
        book.add("Grace \"Amazing\" Hopper", "grace@navy.mil", "+1 2");
        book.add("Linus", "linus@kernel.org", "+358 3");
        book.remove(3);
        book.save(file);
    }
    std::cout << "--- file ---\n" << std::ifstream(file).rdbuf() << "------------\n";

    auto book = ContactBook::load(file);
    std::cout << "loaded " << book.size() << " contacts\n";
    for (const auto& c : book.find_by_name("ADA")) std::cout << "found: [" << c.name << "] " << c.email << '\n';
    for (const auto& c : book.find_by_name("amazing")) std::cout << "found: [" << c.name << "] " << c.email << '\n';
    auto& added = book.add("New Person", "new@x.io", "");
    std::cout << "next id continues after load: " << added.id << '\n';
    fs::remove(file);
}
