#include <iostream>
#include <string>
#include <utility>

template <typename T>
class UniquePtr {
public:
    UniquePtr() = default;
    explicit UniquePtr(T* p) : p_{p} {}
    ~UniquePtr() { delete p_; }

    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept : p_{std::exchange(other.p_, nullptr)} {}
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) reset(std::exchange(other.p_, nullptr));
        return *this;
    }

    T& operator*() const { return *p_; }
    T* operator->() const { return p_; }
    T* get() const { return p_; }
    explicit operator bool() const { return p_ != nullptr; }

    T* release() { return std::exchange(p_, nullptr); } // caller now owns it

    void reset(T* p = nullptr) {
        T* old = std::exchange(p_, p);
        delete old; // delete AFTER updating, in case ~T somehow reaches back into us
    }

private:
    T* p_ = nullptr;
};

template <typename T, typename... Args>
UniquePtr<T> make_unique_ptr(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

struct Player {
    std::string name;
    int level;
    Player(std::string n, int l) : name{std::move(n)}, level{l} { std::cout << "  + " << name << '\n'; }
    ~Player() { std::cout << "  - " << name << '\n'; }
};

int main() {
    auto p = make_unique_ptr<Player>("Ada", 7);
    std::cout << p->name << " level " << (*p).level << '\n';

    UniquePtr<Player> q = std::move(p);
    std::cout << "p is " << (p ? "set" : "empty") << ", q holds " << q->name << '\n';

    q.reset(new Player{"Linus", 3}); // Ada destroyed here
    Player* raw = q.release();       // we own it manually now
    std::cout << "released " << raw->name << '\n';
    delete raw;

    UniquePtr<Player> r = make_unique_ptr<Player>("Grace", 9);
    r = make_unique_ptr<Player>("Bjarne", 10); // Grace destroyed by move-assign
    std::cout << "end of main\n";
}
