#include <iostream>
#include <set>
#include <utility>
#include <vector>

// --- pretend OS API ---
std::set<int> g_open;
int g_next = 100;
int open_resource() {
    int id = g_next++;
    g_open.insert(id);
    std::cout << "  open  " << id << '\n';
    return id;
}
void close_resource(int id) {
    if (g_open.erase(id) == 0) std::cout << "  !!! double close of " << id << '\n';
    else std::cout << "  close " << id << '\n';
}

class Handle {
public:
    Handle() : id_{open_resource()} {}
    ~Handle() {
        if (id_ != -1) close_resource(id_);
    }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    Handle(Handle&& o) noexcept : id_{std::exchange(o.id_, -1)} {}
    Handle& operator=(Handle&& o) noexcept {
        if (this != &o) {
            if (id_ != -1) close_resource(id_);
            id_ = std::exchange(o.id_, -1);
        }
        return *this;
    }
    int id() const { return id_; }

private:
    int id_;
};

void consume(Handle h) { std::cout << "  consuming " << h.id() << '\n'; } // closed when h dies

int main() {
    {
        std::vector<Handle> handles;
        for (int i = 0; i < 3; ++i) handles.emplace_back();

        Handle taken = std::move(handles[1]); // handles[1] now holds -1
        std::cout << "  handles[1] after move: " << handles[1].id() << '\n';
        consume(std::move(taken));

        Handle extra;
        extra = std::move(handles[0]); // closes extra's own handle first
    }
    std::cout << "still open: " << g_open.size() << " (expected 0)\n";
}
