#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Texture {
    std::string file;
    explicit Texture(std::string f) : file{std::move(f)} { std::cout << "  load " << file << '\n'; }
    ~Texture() { std::cout << "  unload " << file << '\n'; }
};

struct Sprite {
    std::shared_ptr<Texture> texture; // many sprites share one texture
};

// --- A reference cycle and how weak_ptr breaks it ---
struct Node {
    std::string name;
    std::shared_ptr<Node> next;  // owning
    std::weak_ptr<Node> prev;    // non-owning: breaks the cycle
    explicit Node(std::string n) : name{std::move(n)} {}
    ~Node() { std::cout << "  ~Node " << name << '\n'; }
};

int main() {
    std::cout << "== shared texture ==\n";
    {
        auto tex = std::make_shared<Texture>("hero.png");
        std::vector<Sprite> sprites{{tex}, {tex}, {tex}};
        std::cout << "  use_count = " << tex.use_count() << '\n'; // 4
        tex.reset();
        std::cout << "  after reset, sprites keep it alive: " << sprites[0].texture.use_count() << '\n';
    } // last owner gone → unload

    std::cout << "== weak_ptr observing ==\n";
    std::weak_ptr<Texture> observer;
    {
        auto tex = std::make_shared<Texture>("enemy.png");
        observer = tex;
        if (auto locked = observer.lock()) std::cout << "  still alive: " << locked->file << '\n';
    }
    std::cout << "  expired? " << std::boolalpha << observer.expired() << '\n';

    std::cout << "== doubly linked nodes (no cycle leak) ==\n";
    {
        auto a = std::make_shared<Node>("A");
        auto b = std::make_shared<Node>("B");
        a->next = b;
        b->prev = a; // if this were shared_ptr, A and B would never be freed
    }
    std::cout << "  end\n";
}
