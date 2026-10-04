// Thread-safe in-memory storage: great for tests, demos, and running the API
// without a database. HTTP requests arrive on many threads, so every access is
// protected by a shared_mutex (many concurrent readers, one writer).
#pragma once

#include <map>
#include <shared_mutex>

#include "repository/task_repository.hpp"

namespace tasks {

class MemoryTaskRepository final : public TaskRepository {
public:
    Task create(const NewTask& task) override;
    std::optional<Task> find(TaskId id) override;
    ListResult list(const ListQuery& query) override;
    std::optional<Task> update(TaskId id, const TaskPatch& patch) override;
    bool remove(TaskId id) override;
    std::string name() const override { return "memory"; }

private:
    mutable std::shared_mutex mutex_;
    std::map<TaskId, Task> tasks_;
    TaskId next_id_ = 1;
};

// Current UTC time as ISO 8601 with second precision: "2026-10-03T14:05:09Z"
std::string now_iso8601();

} // namespace tasks
