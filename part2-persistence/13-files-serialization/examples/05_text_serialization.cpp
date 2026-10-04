// A hand-rolled, versioned, line-based format for a model, with a round-trip test.
// Shows the ideas every serializer needs: escaping, validation, versioning.
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Task {
    int id{};
    std::string title;
    bool done{};
    std::vector<std::string> tags;
    bool operator==(const Task&) const = default;
};

// Format (v2):
//   TASKS v2
//   <id>|<done 0/1>|<title>|<tag1>,<tag2>
// '|' ',' '\' and newlines inside text are escaped with a backslash.
std::string escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '|' || c == ',' || c == '\\') out += '\\';
        if (c == '\n') { out += "\\n"; continue; }
        out += c;
    }
    return out;
}

// Splits on `sep`, honoring backslash escapes.
std::vector<std::string> split_escaped(const std::string& s, char sep) {
    std::vector<std::string> parts{""};
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            parts.back() += (n == 'n') ? '\n' : n;
        } else if (s[i] == sep) {
            parts.emplace_back();
        } else {
            parts.back() += s[i];
        }
    }
    return parts;
}

std::string serialize(const std::vector<Task>& tasks) {
    std::ostringstream out;
    out << "TASKS v2\n";
    for (const auto& t : tasks) {
        out << t.id << '|' << (t.done ? 1 : 0) << '|' << escape(t.title) << '|';
        for (std::size_t i = 0; i < t.tags.size(); ++i) out << (i ? "," : "") << escape(t.tags[i]);
        out << '\n';
    }
    return out.str();
}

std::vector<Task> deserialize(const std::string& text) {
    std::istringstream in(text);
    std::string line;
    if (!std::getline(in, line)) throw std::runtime_error("empty input");
    int version = 0;
    if (line == "TASKS v1") version = 1;      // older files had no tags column
    else if (line == "TASKS v2") version = 2;
    else throw std::runtime_error("bad header: " + line);

    std::vector<Task> tasks;
    int line_no = 1;
    while (std::getline(in, line)) {
        ++line_no;
        if (line.empty()) continue;
        // Split on unescaped '|' only (escapes are preserved for the field-level split)
        std::vector<std::string> cols{""};
        for (std::size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '\\' && i + 1 < line.size()) { cols.back() += line[i]; cols.back() += line[++i]; }
            else if (line[i] == '|') cols.emplace_back();
            else cols.back() += line[i];
        }
        std::size_t expected = version == 1 ? 3 : 4;
        if (cols.size() != expected) throw std::runtime_error("line " + std::to_string(line_no) + ": wrong column count");

        Task t;
        t.id = std::stoi(cols[0]);
        t.done = cols[1] == "1";
        t.title = split_escaped(cols[2], '\0').front(); // '\0' never appears: this just unescapes
        if (version >= 2 && !cols[3].empty()) t.tags = split_escaped(cols[3], ',');
        tasks.push_back(std::move(t));
    }
    return tasks;
}

int main() {
    std::vector<Task> tasks{
        {1, "Learn C++ | the fun way", false, {"study", "c++"}},
        {2, "Ship v1.0", true, {}},
        {3, "Weird, title\nwith newline \\ backslash", false, {"a,b", "c"}},
    };

    std::string text = serialize(tasks);
    std::cout << text << '\n';

    auto back = deserialize(text);
    std::cout << "round trip equal? " << std::boolalpha << (back == tasks) << '\n';

    auto old = deserialize("TASKS v1\n7|0|from an old file\n");
    std::cout << "v1 file loaded: '" << old[0].title << "' with " << old[0].tags.size() << " tags\n";

    try {
        deserialize("GARBAGE\n");
    } catch (const std::exception& e) {
        std::cout << "error: " << e.what() << '\n';
    }
}
