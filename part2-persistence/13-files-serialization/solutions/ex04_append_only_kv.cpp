// An append-only (log-structured) key-value store: the same core idea behind
// write-ahead logs in PostgreSQL, Redis AOF, Kafka, and LSM-tree databases.
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

class KvStore {
public:
    explicit KvStore(fs::path log) : path_{std::move(log)} {
        replay();
        out_.open(path_, std::ios::app | std::ios::binary);
        if (!out_) throw std::runtime_error("cannot open log");
    }

    void set(const std::string& k, const std::string& v) {
        out_ << "SET " << encode(k) << ' ' << encode(v) << '\n';
        out_.flush(); // a real DB would fsync here (that's the "durability" in ACID)
        data_[k] = v;
    }

    void del(const std::string& k) {
        if (!data_.contains(k)) return;
        out_ << "DEL " << encode(k) << '\n';
        out_.flush();
        data_.erase(k);
    }

    std::optional<std::string> get(const std::string& k) const {
        if (auto it = data_.find(k); it != data_.end()) return it->second;
        return std::nullopt;
    }

    void compact() {
        fs::path tmp = path_;
        tmp += ".compact";
        {
            std::ofstream c(tmp, std::ios::binary | std::ios::trunc);
            for (const auto& [k, v] : data_) c << "SET " << encode(k) << ' ' << encode(v) << '\n';
        }
        out_.close();
        fs::rename(tmp, path_); // atomic swap of the whole log
        out_.open(path_, std::ios::app | std::ios::binary);
    }

    std::size_t size() const { return data_.size(); }
    std::size_t skipped_records() const { return skipped_; }

private:
    static std::string encode(const std::string& s) { return std::to_string(s.size()) + ":" + s; }

    // Reads "<len>:<bytes>". Returns nullopt on a truncated/corrupt record.
    static std::optional<std::string> decode(std::istream& in) {
        std::size_t len;
        char colon;
        if (!(in >> len) || !in.get(colon) || colon != ':') return std::nullopt;
        std::string s(len, '\0');
        if (!in.read(s.data(), static_cast<std::streamsize>(len))) return std::nullopt;
        return s;
    }

    void replay() {
        std::ifstream in(path_, std::ios::binary);
        if (!in) return; // new store
        std::string op;
        while (in >> op) {
            auto k = decode(in);
            if (op == "SET") {
                auto v = k ? (in.get(), decode(in)) : std::nullopt; // skip the separating space
                if (!k || !v || in.get() != '\n') { ++skipped_; break; } // torn write at the tail
                data_[*k] = *v;
            } else if (op == "DEL") {
                if (!k || in.get() != '\n') { ++skipped_; break; }
                data_.erase(*k);
            } else {
                ++skipped_;
                break;
            }
        }
    }

    fs::path path_;
    std::ofstream out_;
    std::unordered_map<std::string, std::string> data_;
    std::size_t skipped_ = 0;
};

int main() {
    const fs::path log = fs::temp_directory_path() / "clearn_kv.log";
    fs::remove(log);

    {
        KvStore kv{log};
        kv.set("user:1", "Ada");
        kv.set("user:2", "Grace Hopper");
        kv.set("note", "multi\nline value with spaces");
        for (int i = 0; i < 50; ++i) kv.set("counter", std::to_string(i)); // many overwrites
        kv.del("user:2");
    } // "process exits"

    {
        std::ofstream crash(log, std::ios::app);
        crash << "SET 6:user:3 10:Linu"; // simulated crash mid-write: truncated record
    }

    KvStore kv{log}; // "restart": replay the log
    std::cout << "user:1  = " << kv.get("user:1").value_or("<none>") << '\n';
    std::cout << "user:2  = " << kv.get("user:2").value_or("<none>") << '\n';
    std::cout << "note    = " << kv.get("note").value_or("<none>") << '\n';
    std::cout << "counter = " << kv.get("counter").value_or("<none>") << '\n';
    std::cout << "user:3  = " << kv.get("user:3").value_or("<none>") << "  (torn record ignored, skipped="
              << kv.skipped_records() << ")\n";

    auto before = fs::file_size(log);
    kv.compact();
    std::cout << "log size: " << before << " -> " << fs::file_size(log) << " bytes after compaction\n";

    KvStore again{log};
    std::cout << "after compaction + restart: " << again.size() << " keys, counter = " << again.get("counter").value_or("?") << '\n';
    fs::remove(log);
}
