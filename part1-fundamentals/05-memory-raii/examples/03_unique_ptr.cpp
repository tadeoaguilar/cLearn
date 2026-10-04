#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Enemy {
    std::string name;
    int hp;
    Enemy(std::string n, int h) : name{std::move(n)}, hp{h} { std::cout << "  spawn " << name << '\n'; }
    ~Enemy() { std::cout << "  despawn " << name << '\n'; }
};

// Factory: returns ownership to the caller. Clear and leak-proof.
std::unique_ptr<Enemy> spawn(const std::string& kind) {
    if (kind == "boss") return std::make_unique<Enemy>("Boss", 500);
    return std::make_unique<Enemy>("Goblin", 30);
}

// Takes ownership: the caller must std::move
void bury(std::unique_ptr<Enemy> e) { std::cout << "  burying " << e->name << '\n'; } // freed at end

// Just uses the object: take a reference, not a smart pointer
void hit(Enemy& e, int dmg) { e.hp -= dmg; }

int main() {
    std::cout << "== basic ==\n";
    auto boss = spawn("boss");
    hit(*boss, 120);
    std::cout << "  boss hp = " << boss->hp << '\n';

    std::cout << "== transfer ==\n";
    auto other = std::move(boss); // boss is now empty
    std::cout << "  boss is " << (boss ? "set" : "null") << ", other holds " << other->name << '\n';
    bury(std::move(other));
    std::cout << "  after bury\n";

    std::cout << "== containers own their elements ==\n";
    {
        std::vector<std::unique_ptr<Enemy>> wave;
        for (int i = 0; i < 3; ++i) wave.push_back(spawn("goblin"));
        wave.erase(wave.begin()); // destroys that enemy immediately
        std::cout << "  wave size " << wave.size() << '\n';
    } // remaining enemies destroyed here

    std::cout << "== custom deleter for a C resource ==\n";
    {
        std::unique_ptr<std::FILE, int (*)(std::FILE*)> file{std::fopen("tmp_up.txt", "w"), &std::fclose};
        if (file) std::fputs("hello\n", file.get()); // .get() returns the raw pointer (non-owning)
    } // fclose called automatically
    std::remove("tmp_up.txt");

    std::cout << "== release / reset ==\n";
    auto g = spawn("goblin");
    g.reset(); // destroy now
    std::cout << "  end of main\n";
}
