#include <iostream>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

template <typename K, typename V>
class LRUCache {
public:
    explicit LRUCache(std::size_t capacity) : capacity_{capacity} {}

    std::optional<V> get(const K& key) {
        auto it = index_.find(key);
        if (it == index_.end()) return std::nullopt;
        // Move the node to the front (most recent). splice is O(1) and keeps iterators valid.
        items_.splice(items_.begin(), items_, it->second);
        return it->second->second;
    }

    void put(const K& key, V value) {
        if (auto it = index_.find(key); it != index_.end()) {
            it->second->second = std::move(value);
            items_.splice(items_.begin(), items_, it->second);
            return;
        }
        if (items_.size() == capacity_) {
            const K& lru_key = items_.back().first;
            std::cout << "  (evict " << lru_key << ")\n";
            index_.erase(lru_key);
            items_.pop_back();
        }
        items_.emplace_front(key, std::move(value));
        index_[key] = items_.begin();
    }

    std::size_t size() const { return items_.size(); }

private:
    std::size_t capacity_;
    std::list<std::pair<K, V>> items_; // front = most recently used
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> index_;
};

int main() {
    LRUCache<int, std::string> cache{2};
    cache.put(1, "one");
    cache.put(2, "two");
    std::cout << "get(1) = " << cache.get(1).value_or("<miss>") << '\n'; // 1 is now most recent
    cache.put(3, "three");                                               // evicts 2
    std::cout << "get(2) = " << cache.get(2).value_or("<miss>") << '\n';
    std::cout << "get(3) = " << cache.get(3).value_or("<miss>") << '\n';
    cache.put(1, "uno");                                                 // update, no eviction
    cache.put(4, "four");                                                // evicts 3
    std::cout << "get(1) = " << cache.get(1).value_or("<miss>") << '\n';
    std::cout << "size = " << cache.size() << '\n';
}
