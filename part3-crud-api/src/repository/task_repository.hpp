// The storage abstraction. The service layer depends ONLY on this interface;
// main.cpp decides which implementation to plug in (in-memory or PostgreSQL).
#pragma once

#include <optional>
#include <string>

#include "domain/task.hpp"

namespace tasks {

class TaskRepository {
public:
    virtual ~TaskRepository() = default;

    virtual Task create(const NewTask& task) = 0;
    virtual std::optional<Task> find(TaskId id) = 0;
    virtual ListResult list(const ListQuery& query) = 0;
    // Applies the patch and returns the updated task, or nullopt if `id` doesn't exist.
    virtual std::optional<Task> update(TaskId id, const TaskPatch& patch) = 0;
    // Returns false if `id` doesn't exist.
    virtual bool remove(TaskId id) = 0;

    // For /health and logs: "memory" or "postgres".
    virtual std::string name() const = 0;
    // Throws StorageUnavailableError if the backend can't be reached.
    virtual void ping() {}
};

} // namespace tasks
