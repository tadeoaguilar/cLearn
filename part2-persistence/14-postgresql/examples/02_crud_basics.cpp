// Create / Read / Update / Delete with parameterized queries.
#include <cstdint>
#include <iostream>
#include <optional>
#include <pqxx/pqxx>
#include <string>

#include "db_common.hpp"

struct Book {
    std::int64_t id;
    std::string title;
    std::optional<int> year; // nullable column
    double price;
};

Book row_to_book(const pqxx::row& r) {
    return {r["id"].as<std::int64_t>(), r["title"].as<std::string>(), r["year"].as<std::optional<int>>(),
            r["price"].as<double>()};
}

void print(const Book& b) {
    std::cout << "  #" << b.id << " " << b.title << " (" << (b.year ? std::to_string(*b.year) : "year unknown")
              << ") $" << b.price << '\n';
}

int main() {
    pqxx::connection conn{db::database_url()};

    {   // Schema for this demo
        pqxx::work tx{conn};
        tx.exec("DROP TABLE IF EXISTS demo_books");
        tx.exec(R"(CREATE TABLE demo_books (
                     id    BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                     title TEXT NOT NULL,
                     year  INT,
                     price NUMERIC(10,2) NOT NULL DEFAULT 0))");
        tx.commit();
    }

    // CREATE: INSERT ... RETURNING gives back the generated id
    std::int64_t tour_id{};
    {
        pqxx::work tx{conn};
        tour_id = tx.exec("INSERT INTO demo_books(title, year, price) VALUES ($1, $2, $3) RETURNING id",
                          pqxx::params{"A Tour of C++", 2022, 39.99})
                      .one_row()[0]
                      .as<std::int64_t>();
        tx.exec("INSERT INTO demo_books(title, year, price) VALUES ($1, $2, $3)",
                pqxx::params{"Effective Modern C++", 2014, 44.99});
        tx.exec("INSERT INTO demo_books(title, year, price) VALUES ($1, $2, $3)",
                pqxx::params{"Mystery Manuscript", std::optional<int>{}, 0.0}); // NULL year
        tx.commit();
    }
    std::cout << "inserted 'A Tour of C++' with id " << tour_id << '\n';

    // READ: one row by id, then a list
    {
        pqxx::read_transaction tx{conn};
        auto r = tx.exec("SELECT id, title, year, price FROM demo_books WHERE id = $1", pqxx::params{tour_id});
        if (r.empty()) std::cout << "not found\n";
        else print(row_to_book(r[0]));

        std::cout << "all books:\n";
        for (const auto& row : tx.exec("SELECT id, title, year, price FROM demo_books ORDER BY id")) print(row_to_book(row));
    }

    // UPDATE: affected_rows tells you whether anything matched
    {
        pqxx::work tx{conn};
        auto r = tx.exec("UPDATE demo_books SET price = price * 0.5 WHERE year < $1", pqxx::params{2020});
        std::cout << "discounted " << r.affected_rows() << " book(s)\n";
        auto none = tx.exec("UPDATE demo_books SET title = 'x' WHERE id = $1", pqxx::params{999999});
        std::cout << "update of missing id affected " << none.affected_rows() << " rows (-> 404 in an API)\n";
        tx.commit();
    }

    // DELETE
    {
        pqxx::work tx{conn};
        auto r = tx.exec("DELETE FROM demo_books WHERE year IS NULL");
        std::cout << "deleted " << r.affected_rows() << " book(s) without a year\n";
        tx.commit();
    }

    pqxx::read_transaction tx{conn};
    std::cout << "remaining: " << tx.query_value<int>("SELECT count(*) FROM demo_books") << '\n';
    for (auto [title, price] : tx.query<std::string, double>("SELECT title, price FROM demo_books ORDER BY title"))
        std::cout << "  " << title << " $" << price << '\n';
}
