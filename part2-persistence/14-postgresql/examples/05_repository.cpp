// The repository pattern: business logic talks to an interface; Postgres and
// in-memory implementations are interchangeable. Database errors are translated
// into domain errors so nothing above the repository depends on libpqxx.
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>
#include <vector>

#include "db_common.hpp"

// ---------- domain ----------
struct User {
    std::int64_t id{};
    std::string email;
    std::string name;
};

struct NewUser {
    std::string email;
    std::string name;
};

class DuplicateEmail : public std::runtime_error {
public:
    explicit DuplicateEmail(const std::string& email) : std::runtime_error("email already registered: " + email) {}
};

class UserRepository {
public:
    virtual ~UserRepository() = default;
    virtual User create(const NewUser& u) = 0;
    virtual std::optional<User> find(std::int64_t id) = 0;
    virtual std::vector<User> list() = 0;
    virtual bool rename(std::int64_t id, const std::string& name) = 0;
    virtual bool remove(std::int64_t id) = 0;
};

// ---------- PostgreSQL implementation ----------
class PgUserRepository final : public UserRepository {
public:
    explicit PgUserRepository(pqxx::connection& conn) : conn_{conn} {
        pqxx::work tx{conn_};
        tx.exec(R"(CREATE TABLE IF NOT EXISTS demo_repo_users (
                     id    BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                     email TEXT NOT NULL UNIQUE,
                     name  TEXT NOT NULL))");
        tx.commit();
    }

    User create(const NewUser& u) override {
        try {
            pqxx::work tx{conn_};
            auto row = tx.exec("INSERT INTO demo_repo_users(email, name) VALUES ($1, $2) RETURNING id, email, name",
                               pqxx::params{u.email, u.name})
                           .one_row();
            tx.commit();
            return to_user(row);
        } catch (const pqxx::unique_violation&) {
            throw DuplicateEmail(u.email); // translate infrastructure error -> domain error
        }
    }

    std::optional<User> find(std::int64_t id) override {
        pqxx::read_transaction tx{conn_};
        auto r = tx.exec("SELECT id, email, name FROM demo_repo_users WHERE id = $1", pqxx::params{id});
        if (r.empty()) return std::nullopt;
        return to_user(r[0]);
    }

    std::vector<User> list() override {
        pqxx::read_transaction tx{conn_};
        std::vector<User> out;
        for (const auto& row : tx.exec("SELECT id, email, name FROM demo_repo_users ORDER BY id")) out.push_back(to_user(row));
        return out;
    }

    bool rename(std::int64_t id, const std::string& name) override {
        pqxx::work tx{conn_};
        auto r = tx.exec("UPDATE demo_repo_users SET name = $1 WHERE id = $2", pqxx::params{name, id});
        tx.commit();
        return r.affected_rows() == 1;
    }

    bool remove(std::int64_t id) override {
        pqxx::work tx{conn_};
        auto r = tx.exec("DELETE FROM demo_repo_users WHERE id = $1", pqxx::params{id});
        tx.commit();
        return r.affected_rows() == 1;
    }

private:
    static User to_user(const pqxx::row& r) {
        return {r["id"].as<std::int64_t>(), r["email"].as<std::string>(), r["name"].as<std::string>()};
    }
    pqxx::connection& conn_;
};

// ---------- in-memory implementation (tests, demos, prototyping) ----------
class InMemoryUserRepository final : public UserRepository {
public:
    User create(const NewUser& u) override {
        if (std::ranges::any_of(users_, [&](const auto& kv) { return kv.second.email == u.email; })) throw DuplicateEmail(u.email);
        User user{next_id_++, u.email, u.name};
        users_[user.id] = user;
        return user;
    }
    std::optional<User> find(std::int64_t id) override {
        if (auto it = users_.find(id); it != users_.end()) return it->second;
        return std::nullopt;
    }
    std::vector<User> list() override {
        std::vector<User> out;
        for (const auto& [id, u] : users_) out.push_back(u);
        return out;
    }
    bool rename(std::int64_t id, const std::string& name) override {
        auto it = users_.find(id);
        if (it == users_.end()) return false;
        it->second.name = name;
        return true;
    }
    bool remove(std::int64_t id) override { return users_.erase(id) == 1; }

private:
    std::map<std::int64_t, User> users_;
    std::int64_t next_id_ = 1;
};

// ---------- business logic: knows nothing about SQL ----------
class UserService {
public:
    explicit UserService(UserRepository& repo) : repo_{repo} {}

    User register_user(std::string email, std::string name) {
        if (email.find('@') == std::string::npos) throw std::invalid_argument("invalid email");
        std::ranges::transform(email, email.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return repo_.create({email, name});
    }
    UserRepository& repo() { return repo_; }

private:
    UserRepository& repo_;
};

void demo(const char* label, UserRepository& repo) {
    std::cout << "=== " << label << " ===\n";
    UserService svc{repo};
    auto ada = svc.register_user("Ada@Example.com", "Ada");
    svc.register_user("grace@example.com", "Grace");
    try {
        svc.register_user("ada@example.com", "Impostor");
    } catch (const DuplicateEmail& e) {
        std::cout << "  rejected: " << e.what() << '\n';
    }
    svc.repo().rename(ada.id, "Ada Lovelace");
    for (const auto& u : svc.repo().list()) std::cout << "  #" << u.id << " " << u.name << " <" << u.email << ">\n";
    std::cout << "  find(999): " << (svc.repo().find(999) ? "found" : "not found") << '\n';
}

int main() {
    InMemoryUserRepository memory;
    demo("in-memory", memory);

    pqxx::connection conn{db::database_url()};
    {
        pqxx::work tx{conn};
        tx.exec("DROP TABLE IF EXISTS demo_repo_users");
        tx.commit();
    }
    PgUserRepository pg{conn};
    demo("postgres", pg); // identical behavior, different storage
}
