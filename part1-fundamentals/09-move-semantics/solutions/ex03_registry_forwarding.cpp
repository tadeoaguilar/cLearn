#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

template <typename T>
class Registry {
public:
    template <typename... Args>
    T& create(Args&&... args) {
        items_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return *items_.back();
    }
    std::size_t size() const { return items_.size(); }

private:
    std::vector<std::unique_ptr<T>> items_;
};

struct Monster {
    std::string name;
    int id;
    Monster(std::string&& n, int& counter) : name{std::move(n)}, id{++counter} {} // requires rvalue + lvalue ref
};

int main() {
    Registry<Monster> reg;
    int counter = 0;
    auto& orc = reg.create(std::string{"orc"}, counter);   // string forwarded as rvalue
    auto& troll = reg.create(std::string{"troll"}, counter); // counter forwarded as int&
    std::cout << orc.name << '#' << orc.id << ", " << troll.name << '#' << troll.id << '\n';
    std::cout << "counter was modified through the forwarded reference: " << counter << '\n';
    std::cout << "registry size: " << reg.size() << '\n';
}
