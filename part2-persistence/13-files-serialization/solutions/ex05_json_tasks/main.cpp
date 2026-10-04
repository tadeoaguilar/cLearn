// A tiny JSON-backed task manager: the persistence layer of the CRUD API in miniature.
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

struct Task {
    int id{};
    std::string title;
    bool done{};
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Task, id, title, done)

class TaskFile {
public:
    explicit TaskFile(fs::path p) : path_{std::move(p)} { load(); }

    const Task& add(std::string title) {
        tasks_.push_back({next_id_++, std::move(title), false});
        save();
        return tasks_.back();
    }
    bool mark_done(int id) {
        auto it = std::ranges::find(tasks_, id, &Task::id);
        if (it == tasks_.end()) return false;
        it->done = true;
        save();
        return true;
    }
    bool remove(int id) {
        if (std::erase_if(tasks_, [id](const Task& t) { return t.id == id; }) == 0) return false;
        save();
        return true;
    }
    const std::vector<Task>& all() const { return tasks_; }

private:
    void load() {
        if (!fs::exists(path_)) return;
        try {
            json j = json::parse(std::ifstream(path_));
            tasks_ = j.at("tasks").get<std::vector<Task>>();
            next_id_ = j.value("next_id", 1);
        } catch (const json::exception& e) {
            fs::path backup = path_;
            backup += ".corrupt";
            fs::copy_file(path_, backup, fs::copy_options::overwrite_existing);
            std::cerr << "warning: " << path_.filename() << " is corrupt (" << e.what() << "); starting empty, copy kept at "
                      << backup.filename() << '\n';
            tasks_.clear();
            next_id_ = 1;
        }
    }

    void save() const {
        fs::path tmp = path_;
        tmp += ".tmp";
        {
            std::ofstream out(tmp, std::ios::trunc);
            out << json{{"next_id", next_id_}, {"tasks", tasks_}}.dump(2);
            if (!out) throw std::runtime_error("save failed");
        }
        fs::rename(tmp, path_);
    }

    fs::path path_;
    std::vector<Task> tasks_;
    int next_id_ = 1;
};

void run(TaskFile& tf, const std::string& line) {
    std::istringstream in(line);
    std::string cmd;
    in >> cmd;
    std::cout << "> " << line << '\n';
    if (cmd == "add") {
        std::string title;
        std::getline(in >> std::ws, title);
        std::cout << "  added #" << tf.add(title).id << '\n';
    } else if (cmd == "done" || cmd == "remove") {
        int id{};
        in >> id;
        bool ok = cmd == "done" ? tf.mark_done(id) : tf.remove(id);
        std::cout << (ok ? "  ok\n" : "  no such task\n");
    } else if (cmd == "list") {
        for (const auto& t : tf.all()) std::cout << "  [" << (t.done ? 'x' : ' ') << "] #" << t.id << ' ' << t.title << '\n';
    }
}

int main() {
    const fs::path file = fs::temp_directory_path() / "clearn_tasks.json";
    fs::remove(file);
    {
        TaskFile tf{file};
        for (const char* cmd : {"add Learn persistence", "add Build CRUD API", "add Make a game", "done 1", "remove 3", "list"})
            run(tf, cmd);
    }
    std::cout << "--- " << file.filename() << " ---\n" << std::ifstream(file).rdbuf() << "\n---\n";

    TaskFile reopened{file};
    run(reopened, "add Survives restarts");
    run(reopened, "list");

    std::ofstream(file) << "{ this is not json";
    TaskFile recovered{file};
    run(recovered, "list");
    fs::remove(file);
    fs::remove(fs::path(file) += ".corrupt");
}
