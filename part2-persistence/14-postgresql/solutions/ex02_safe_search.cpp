#include <algorithm>
#include <cstdint>
#include <iostream>
#include <optional>
#include <pqxx/pqxx>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "db_common.hpp"

struct Book {
    std::int64_t id;
    std::string title;
    int year;
    double price;
};

struct BookQuery {
    std::optional<std::string> title_contains;
    std::optional<int> min_year;
    std::string sort_by = "title";
    bool descending = false;
    int limit = 10, offset = 0;
};

// Column names can't be bound as parameters, so map user input -> known-safe SQL.
std::string_view sort_column(const std::string& s) {
    if (s == "title") return "title";
    if (s == "year") return "year";
    if (s == "price") return "price";
    throw std::invalid_argument("cannot sort by '" + s + "'");
}

std::vector<Book> search(pqxx::connection& conn, const BookQuery& q) {
    std::string sql = "SELECT id, title, year, price FROM ex_books WHERE true";
    pqxx::params params;
    int n = 0;
    auto next = [&n] { return "$" + std::to_string(++n); };

    if (q.title_contains) {
        sql += " AND title ILIKE " + next();
        params.append("%" + *q.title_contains + "%"); // the VALUE is a parameter, even the wildcards
    }
    if (q.min_year) {
        sql += " AND year >= " + next();
        params.append(*q.min_year);
    }
    sql += " ORDER BY ";
    sql += sort_column(q.sort_by); // whitelisted
    sql += q.descending ? " DESC" : " ASC";
    sql += ", id"; // deterministic order for stable pagination
    sql += " LIMIT " + next();
    params.append(std::clamp(q.limit, 1, 100));
    sql += " OFFSET " + next();
    params.append(std::max(q.offset, 0));

    pqxx::read_transaction tx{conn};
    std::vector<Book> out;
    for (const auto& r : tx.exec(sql, params))
        out.push_back({r[0].as<std::int64_t>(), r[1].as<std::string>(), r[2].as<int>(), r[3].as<double>()});
    return out;
}

void show(const char* label, const std::vector<Book>& books) {
    std::cout << label << '\n';
    for (const auto& b : books) std::cout << "  " << b.year << "  $" << b.price << "  " << b.title << '\n';
}

int main() {
    pqxx::connection conn{db::database_url()};
    {
        pqxx::work tx{conn};
        tx.exec("DROP TABLE IF EXISTS ex_books CASCADE");
        tx.exec("CREATE TABLE ex_books (id BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY, title TEXT NOT NULL, year INT NOT NULL, price NUMERIC(10,2) NOT NULL)");
        tx.exec(R"(INSERT INTO ex_books(title, year, price) VALUES
                   ('A Tour of C++', 2022, 39.99), ('Effective Modern C++', 2014, 44.99),
                   ('Effective STL', 2001, 29.99), ('C++ Concurrency in Action', 2019, 49.99),
                   ('Game Programming Patterns', 2014, 0), ('C++ Templates', 2017, 59.99))");
        tx.commit();
    }

    show("All, by title:", search(conn, {}));
    show("'effective', newest first:", search(conn, {.title_contains = "effective", .sort_by = "year", .descending = true}));
    show("year >= 2015, by price desc, page 1 (2 per page):", search(conn, {.min_year = 2015, .sort_by = "price", .descending = true, .limit = 2}));
    show("... page 2:", search(conn, {.min_year = 2015, .sort_by = "price", .descending = true, .limit = 2, .offset = 2}));
    show("title containing a quote (harmless):", search(conn, {.title_contains = "'; DROP TABLE ex_books; --"}));

    try {
        search(conn, {.sort_by = "price; DROP TABLE ex_books"});
    } catch (const std::invalid_argument& e) {
        std::cout << "rejected: " << e.what() << '\n';
    }
    pqxx::read_transaction tx{conn};
    std::cout << "table still has " << tx.query_value<int>("SELECT count(*) FROM ex_books") << " rows\n";
}
