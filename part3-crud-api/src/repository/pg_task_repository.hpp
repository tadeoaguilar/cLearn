// PostgreSQL-backed storage. All SQL in the application lives in this file.
#pragma once

#include "infra/connection_pool.hpp"
#include "repository/task_repository.hpp"

namespace tasks {

class PgTaskRepository final : public TaskRepository {
public:
    explicit PgTaskRepository(infra::ConnectionPool& pool) : pool_{pool} {}

    Task create(const NewTask& task) override;
    std::optional<Task> find(TaskId id) override;
    ListResult list(const ListQuery& query) override;
    std::optional<Task> update(TaskId id, const TaskPatch& patch) override;
    bool remove(TaskId id) override;
    std::string name() const override { return "postgres"; }
    void ping() override;

private:
    // Runs `fn` with a leased connection, translating connection failures into
    // StorageUnavailableError so upper layers never see libpqxx types.
    template <typename F>
    auto with_connection(F&& fn);

    infra::ConnectionPool& pool_;
};

} // namespace tasks
