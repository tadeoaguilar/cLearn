#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <pqxx/pqxx>
#include <set>
#include <string>
#include <vector>

#include "db_common.hpp"

struct Note {
    std::int64_t id{};
    std::string title;
    std::string body;
    bool operator==(const Note&) const = default;
};

class NoteRepository {
public:
    virtual ~NoteRepository() = default;
    virtual Note create(const std::string& title, const std::string& body) = 0;
    virtual std::optional<Note> get(std::int64_t id) = 0;
    virtual std::vector<Note> list(int limit, int offset) = 0; // ordered by id
    virtual bool update(const Note& n) = 0;
    virtual bool remove(std::int64_t id) = 0;
};

class InMemoryNotes final : public NoteRepository {
public:
    Note create(const std::string& t, const std::string& b) override {
        Note n{next_++, t, b};
        notes_[n.id] = n;
        return n;
    }
    std::optional<Note> get(std::int64_t id) override {
        auto it = notes_.find(id);
        return it == notes_.end() ? std::nullopt : std::optional{it->second};
    }
    std::vector<Note> list(int limit, int offset) override {
        std::vector<Note> out;
        int i = 0;
        for (const auto& [id, n] : notes_) {
            if (i++ < offset) continue;
            if (static_cast<int>(out.size()) == limit) break;
            out.push_back(n);
        }
        return out;
    }
    bool update(const Note& n) override {
        auto it = notes_.find(n.id);
        if (it == notes_.end()) return false;
        it->second = n;
        return true;
    }
    bool remove(std::int64_t id) override { return notes_.erase(id) == 1; }

private:
    std::map<std::int64_t, Note> notes_;
    std::int64_t next_ = 1;
};

class PgNotes final : public NoteRepository {
public:
    explicit PgNotes(pqxx::connection& c) : c_{c} {
        pqxx::work tx{c_};
        tx.exec("DROP TABLE IF EXISTS ex_notes");
        tx.exec("CREATE TABLE ex_notes (id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY, title TEXT NOT NULL, body TEXT NOT NULL)");
        tx.commit();
    }
    Note create(const std::string& t, const std::string& b) override {
        pqxx::work tx{c_};
        auto row = tx.exec("INSERT INTO ex_notes(title, body) VALUES ($1, $2) RETURNING id, title, body", pqxx::params{t, b}).one_row();
        tx.commit();
        return to_note(row);
    }
    std::optional<Note> get(std::int64_t id) override {
        pqxx::read_transaction tx{c_};
        auto r = tx.exec("SELECT id, title, body FROM ex_notes WHERE id = $1", pqxx::params{id});
        return r.empty() ? std::nullopt : std::optional{to_note(r[0])};
    }
    std::vector<Note> list(int limit, int offset) override {
        pqxx::read_transaction tx{c_};
        std::vector<Note> out;
        for (const auto& r : tx.exec("SELECT id, title, body FROM ex_notes ORDER BY id LIMIT $1 OFFSET $2", pqxx::params{limit, offset}))
            out.push_back(to_note(r));
        return out;
    }
    bool update(const Note& n) override {
        pqxx::work tx{c_};
        auto r = tx.exec("UPDATE ex_notes SET title = $1, body = $2 WHERE id = $3", pqxx::params{n.title, n.body, n.id});
        tx.commit();
        return r.affected_rows() == 1;
    }
    bool remove(std::int64_t id) override {
        pqxx::work tx{c_};
        auto r = tx.exec("DELETE FROM ex_notes WHERE id = $1", pqxx::params{id});
        tx.commit();
        return r.affected_rows() == 1;
    }

private:
    static Note to_note(const pqxx::row& r) { return {r[0].as<std::int64_t>(), r[1].as<std::string>(), r[2].as<std::string>()}; }
    pqxx::connection& c_;
};

// ---------- the contract: behavior every NoteRepository must have ----------
int run_contract_tests(const std::string& name, NoteRepository& repo) {
    int failures = 0;
    auto check = [&](bool ok, const char* what) {
        std::cout << (ok ? "  [ok]   " : "  [FAIL] ") << what << '\n';
        failures += !ok;
    };
    std::cout << name << ":\n";

    auto a = repo.create("first", "hello");
    auto b = repo.create("second", "world");
    check(a.id != b.id, "ids are unique");
    check(repo.get(a.id) == a, "get after create returns the same data");
    check(!repo.get(999'999).has_value(), "get of a missing id returns nullopt");

    Note changed = a;
    changed.body = "edited";
    check(repo.update(changed), "update existing returns true");
    check(repo.get(a.id)->body == "edited", "update is visible");
    check(!repo.update(Note{999'999, "x", "y"}), "update missing returns false");

    for (int i = 0; i < 5; ++i) repo.create("n" + std::to_string(i), "");
    auto page1 = repo.list(3, 0), page2 = repo.list(3, 3), page3 = repo.list(3, 6);
    check(page1.size() == 3 && page2.size() == 3 && page3.size() == 1, "pagination sizes 3/3/1 for 7 notes");
    std::set<std::int64_t> seen;
    for (const auto* page : {&page1, &page2, &page3})
        for (const auto& n : *page) seen.insert(n.id);
    check(seen.size() == 7, "pages don't overlap");
    check(std::ranges::is_sorted(page1, {}, &Note::id), "list is ordered by id");

    check(repo.remove(b.id), "remove existing returns true");
    check(!repo.get(b.id), "removed note is gone");
    check(!repo.remove(b.id), "remove twice returns false");
    return failures;
}

int main() {
    InMemoryNotes mem;
    pqxx::connection conn{db::database_url()};
    PgNotes pg{conn};
    int failures = run_contract_tests("InMemoryNotes", mem) + run_contract_tests("PgNotes", pg);
    std::cout << (failures == 0 ? "all contract tests passed\n" : "some contract tests FAILED\n");
    return failures == 0 ? 0 : 1;
}
