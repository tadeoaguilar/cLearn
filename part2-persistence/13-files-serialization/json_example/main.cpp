// nlohmann/json tour: building JSON, structs <-> JSON, optional fields, files, errors.
// Built by CMake (it downloads the library):  ./build/.../ch13_json_example
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

enum class Priority { Low, Medium, High };
// Store enums as readable strings, not numbers
NLOHMANN_JSON_SERIALIZE_ENUM(Priority, {{Priority::Low, "low"}, {Priority::Medium, "medium"}, {Priority::High, "high"}})

struct Task {
    int id{};
    std::string title;
    bool done{};
    Priority priority{Priority::Medium};
    std::vector<std::string> tags;
    std::optional<std::string> due; // optional field: omitted from JSON when empty
};

// Hand-written converters give full control (the macro below is the shortcut)
void to_json(json& j, const Task& t) {
    j = json{{"id", t.id}, {"title", t.title}, {"done", t.done}, {"priority", t.priority}, {"tags", t.tags}};
    if (t.due) j["due"] = *t.due;
}

void from_json(const json& j, Task& t) {
    j.at("id").get_to(t.id);       // at() throws if the key is missing
    j.at("title").get_to(t.title);
    t.done = j.value("done", false); // value() supplies a default
    t.priority = j.value("priority", Priority::Medium);
    t.tags = j.value("tags", std::vector<std::string>{});
    if (j.contains("due") && !j["due"].is_null()) t.due = j["due"].get<std::string>();
    else t.due.reset();
}

// The macro shortcut: generates to_json/from_json for simple structs
struct Settings {
    std::string theme = "dark";
    double volume = 0.8;
    bool fullscreen = false;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Settings, theme, volume, fullscreen)

int main() {
    // 1) Building JSON dynamically
    json doc = {{"app", "cLearn"}, {"version", 1}, {"features", {"json", "files"}}};
    doc["nested"]["answer"] = 42;
    std::cout << doc.dump(2) << "\n\n";

    // 2) Structs -> JSON -> text -> JSON -> structs
    std::vector<Task> tasks{{1, "Write chapter 13", true, Priority::High, {"docs"}, "2026-10-10"},
                            {2, "Build the API", false, Priority::Medium, {"code", "api"}, std::nullopt}};
    json j = tasks; // vector<Task> converts automatically thanks to to_json
    std::string text = j.dump(2);
    std::cout << text << "\n\n";

    auto back = json::parse(text).get<std::vector<Task>>();
    std::cout << "loaded " << back.size() << " tasks; #2 due? " << (back[1].due ? *back[1].due : "none") << "\n\n";

    // 3) Save & load a file
    fs::path file = fs::temp_directory_path() / "clearn_settings.json";
    std::ofstream(file) << json(Settings{"light", 0.5, true}).dump(2);
    Settings s = json::parse(std::ifstream(file)).get<Settings>();
    std::cout << "settings: theme=" << s.theme << " volume=" << s.volume << " fullscreen=" << s.fullscreen << '\n';

    // Missing keys fall back to defaults with the _WITH_DEFAULT macro
    Settings partial = json::parse(R"({"volume": 0.1})").get<Settings>();
    std::cout << "partial: theme=" << partial.theme << " volume=" << partial.volume << "\n\n";
    fs::remove(file);

    // 4) Error handling: never trust input
    for (const char* bad : {R"({"id": 1})", R"({"id": "one", "title": "x"})", R"({not json})"}) {
        try {
            auto t = json::parse(bad).get<Task>();
            std::cout << "parsed?! " << t.id << '\n';
        } catch (const json::exception& e) {
            std::cout << "rejected: " << e.what() << '\n';
        }
    }
}
