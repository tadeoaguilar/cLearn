#include <cstdint>
#include <iomanip>
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "db_common.hpp"

int main() {
    pqxx::connection conn{db::database_url()};
    {
        pqxx::work tx{conn};
        tx.exec("DROP TABLE IF EXISTS ex_books, ex_authors");
        tx.exec(R"(CREATE TABLE ex_authors (
                     id   BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                     name TEXT NOT NULL UNIQUE))");
        tx.exec(R"(CREATE TABLE ex_books (
                     id        BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                     author_id BIGINT NOT NULL REFERENCES ex_authors(id) ON DELETE CASCADE,
                     title     TEXT NOT NULL,
                     year      INT NOT NULL))");
        tx.commit();
    }

    // Insert everything atomically
    {
        pqxx::work tx{conn};
        auto add_author = [&](const std::string& name) {
            return tx.exec("INSERT INTO ex_authors(name) VALUES ($1) RETURNING id", pqxx::params{name}).one_row()[0].as<std::int64_t>();
        };
        auto add_book = [&](std::int64_t author, const std::string& title, int year) {
            tx.exec("INSERT INTO ex_books(author_id, title, year) VALUES ($1, $2, $3)", pqxx::params{author, title, year});
        };
        auto stroustrup = add_author("Bjarne Stroustrup");
        auto meyers = add_author("Scott Meyers");
        add_author("Herb Sutter"); // no books yet
        add_book(stroustrup, "The C++ Programming Language", 2013);
        add_book(stroustrup, "A Tour of C++", 2022);
        add_book(stroustrup, "Programming: Principles and Practice", 2024);
        add_book(meyers, "Effective C++", 2005);
        add_book(meyers, "Effective STL", 2001);
        add_book(meyers, "Effective Modern C++", 2014);
        tx.commit();
    }

    auto report = [&] {
        pqxx::read_transaction tx{conn};
        std::cout << "Books by author:\n";
        for (auto [author, title, year] : tx.query<std::string, std::string, int>(
                 "SELECT a.name, b.title, b.year FROM ex_books b JOIN ex_authors a ON a.id = b.author_id "
                 "ORDER BY a.name, b.year"))
            std::cout << "  " << std::left << std::setw(20) << author << year << "  " << title << '\n';

        std::cout << "Count per author:\n";
        for (auto [author, n] : tx.query<std::string, int>(
                 "SELECT a.name, count(b.id)::int FROM ex_authors a LEFT JOIN ex_books b ON b.author_id = a.id "
                 "GROUP BY a.name ORDER BY count(b.id) DESC, a.name"))
            std::cout << "  " << std::left << std::setw(20) << author << n << '\n';
    };
    report();

    {
        pqxx::work tx{conn};
        auto r = tx.exec("DELETE FROM ex_authors WHERE name = $1", pqxx::params{"Scott Meyers"});
        tx.commit();
        std::cout << "\nDeleted " << r.affected_rows() << " author (books cascade):\n";
    }
    report();
}
