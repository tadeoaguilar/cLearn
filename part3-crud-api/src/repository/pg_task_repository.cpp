#include "repository/pg_task_repository.hpp"

#include <string>

#include "domain/errors.hpp"

namespace tasks {

namespace {

// Format timestamps in SQL so both repositories return identical ISO 8601 strings.
constexpr const char* kColumns = R"(id, title, description, status, priority, due_date::text,
    to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD"T"HH24:MI:SS"Z"'),
    to_char(updated_at AT TIME ZONE 'UTC', 'YYYY-MM-DD"T"HH24:MI:SS"Z"'))";

Task to_task(const pqxx::row& r) {
    Task t;
    t.id = r[0].as<TaskId>();
    t.title = r[1].as<std::string>();
    t.description = r[2].as<std::string>();
    t.status = parse_status(r[3].as<std::string>()).value_or(TaskStatus::Todo);
    t.priority = parse_priority(r[4].as<std::string>()).value_or(Priority::Medium);
    t.due_date = r[5].as<std::optional<std::string>>();
    t.created_at = r[6].as<std::string>();
    t.updated_at = r[7].as<std::string>();
    return t;
}

// ORDER BY can't be a parameter: map the enum to a fixed, known-safe SQL fragment.
const char* order_column(SortField f) {
    switch (f) {
        case SortField::CreatedAt: return "created_at";
        case SortField::UpdatedAt: return "updated_at";
        case SortField::DueDate: return "due_date";
        case SortField::Priority: return "priority_rank";
        case SortField::Title: return "title";
    }
    return "created_at";
}

std::string to_db(TaskStatus s) { return std::string(to_string(s)); }
std::string to_db(Priority p) { return std::string(to_string(p)); }

} // namespace

template <typename F>
auto PgTaskRepository::with_connection(F&& fn) {
    auto lease = pool_.acquire();
    try {
        return fn(*lease);
    } catch (const pqxx::broken_connection& e) {
        throw StorageUnavailableError(e.what());
    }
}

void PgTaskRepository::ping() {
    with_connection([](pqxx::connection& c) {
        pqxx::nontransaction tx{c};
        tx.exec("SELECT 1");
        return 0;
    });
}

Task PgTaskRepository::create(const NewTask& in) {
    return with_connection([&](pqxx::connection& c) {
        pqxx::work tx{c};
        auto row = tx.exec(std::string("INSERT INTO tasks (title, description, status, priority, due_date) "
                                       "VALUES ($1, $2, $3, $4, $5::date) RETURNING ") + kColumns,
                           pqxx::params{in.title, in.description, to_db(in.status), to_db(in.priority), in.due_date})
                       .one_row();
        tx.commit();
        return to_task(row);
    });
}

std::optional<Task> PgTaskRepository::find(TaskId id) {
    return with_connection([&](pqxx::connection& c) -> std::optional<Task> {
        pqxx::read_transaction tx{c};
        auto r = tx.exec(std::string("SELECT ") + kColumns + " FROM tasks WHERE id = $1", pqxx::params{id});
        if (r.empty()) return std::nullopt;
        return to_task(r[0]);
    });
}

ListResult PgTaskRepository::list(const ListQuery& q) {
    // Build the WHERE clause dynamically, but every VALUE is a bound parameter.
    std::string where = " WHERE true";
    pqxx::params params;
    int n = 0;
    auto placeholder = [&n] { return "$" + std::to_string(++n); };

    if (q.status) {
        where += " AND status = " + placeholder();
        params.append(to_db(*q.status));
    }
    if (q.priority) {
        where += " AND priority = " + placeholder();
        params.append(to_db(*q.priority));
    }
    if (q.search) {
        auto p = placeholder();
        where += " AND (title ILIKE " + p + " OR description ILIKE " + p + ")";
        std::string escaped; // escape LIKE wildcards in user input so "50%" means a literal %
        for (char ch : *q.search) {
            if (ch == '%' || ch == '_' || ch == '\\') escaped += '\\';
            escaped += ch;
        }
        params.append("%" + escaped + "%");
    }

    std::string order = std::string(" ORDER BY ") + order_column(q.sort) + (q.descending ? " DESC" : " ASC") +
                        " NULLS LAST, id" + (q.descending ? " DESC" : " ASC");
    // Two separate statements on purpose: in `a + f() + b + f()` C++ does NOT specify which
    // f() runs first. GCC evaluated the right one first and produced "LIMIT $2 OFFSET $1".
    std::string page = " LIMIT " + placeholder();
    page += " OFFSET " + placeholder();
    pqxx::params page_params = params;
    page_params.append(q.limit);
    page_params.append(q.offset);

    return with_connection([&](pqxx::connection& c) {
        // REPEATABLE READ: the count and the page come from the same snapshot.
        pqxx::transaction<pqxx::isolation_level::repeatable_read, pqxx::write_policy::read_only> tx{c};
        ListResult result;
        result.total = tx.query_value<std::int64_t>("SELECT count(*) FROM tasks" + where, params);
        for (const auto& row : tx.exec(std::string("SELECT ") + kColumns + " FROM tasks" + where + order + page, page_params))
            result.items.push_back(to_task(row));
        return result;
    });
}

std::optional<Task> PgTaskRepository::update(TaskId id, const TaskPatch& p) {
    std::string set = "updated_at = now()";
    pqxx::params params;
    int n = 0;
    auto assign = [&](const char* column, auto value, const char* cast = "") {
        set += std::string(", ") + column + " = $" + std::to_string(++n) + cast;
        params.append(std::move(value));
    };
    if (p.title) assign("title", *p.title);
    if (p.description) assign("description", *p.description);
    if (p.status) assign("status", to_db(*p.status));
    if (p.priority) assign("priority", to_db(*p.priority));
    if (p.due_date) assign("due_date", *p.due_date, "::date"); // inner nullopt → SQL NULL (clears it)
    params.append(id);
    std::string sql = "UPDATE tasks SET " + set + " WHERE id = $" + std::to_string(++n) + " RETURNING " + kColumns;

    return with_connection([&](pqxx::connection& c) -> std::optional<Task> {
        pqxx::work tx{c};
        auto r = tx.exec(sql, params);
        tx.commit();
        if (r.empty()) return std::nullopt;
        return to_task(r[0]);
    });
}

bool PgTaskRepository::remove(TaskId id) {
    return with_connection([&](pqxx::connection& c) {
        pqxx::work tx{c};
        auto r = tx.exec("DELETE FROM tasks WHERE id = $1", pqxx::params{id});
        tx.commit();
        return r.affected_rows() == 1;
    });
}

} // namespace tasks
