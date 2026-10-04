#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

struct Asset {
    std::string name;
    explicit Asset(std::string n) : name{std::move(n)} { std::cout << "  [load] " << name << '\n'; }
    ~Asset() { std::cout << "  [free] " << name << '\n'; }
};

class AssetCache {
public:
    std::shared_ptr<Asset> get(const std::string& name) {
        if (auto it = cache_.find(name); it != cache_.end()) {
            if (auto alive = it->second.lock()) return alive; // still in use somewhere
        }
        auto asset = std::make_shared<Asset>(name);
        cache_[name] = asset; // store weak reference only
        ++loads_;
        return asset;
    }

    int loads() const { return loads_; }

private:
    std::unordered_map<std::string, std::weak_ptr<Asset>> cache_;
    int loads_ = 0;
};

int main() {
    AssetCache cache;
    {
        auto a = cache.get("hero.png");
        auto b = cache.get("hero.png"); // shared, no reload
        auto c = cache.get("tree.png");
        std::cout << "  a and b same object? " << std::boolalpha << (a == b) << '\n';
    } // all released → assets freed

    auto again = cache.get("hero.png"); // reloaded because it was freed
    std::cout << "total loads = " << cache.loads() << " (expected 3)\n";
}
