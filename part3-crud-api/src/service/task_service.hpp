// Business logic: validation, normalization and rules. Knows nothing about
// HTTP or SQL, so it is trivially unit-testable with MemoryTaskRepository.
#pragma once

#include "domain/errors.hpp"
#include "domain/task.hpp"
#include "repository/task_repository.hpp"

namespace tasks {

class TaskService {
public:
    explicit TaskService(TaskRepository& repo) : repo_{repo} {}

    Task create(NewTask input);                       // throws ValidationError
    Task get(TaskId id);                              // throws NotFoundError
    ListResult list(ListQuery query);                 // throws ValidationError
    Task replace(TaskId id, NewTask input);           // PUT: throws ValidationError / NotFoundError
    Task patch(TaskId id, TaskPatch patch);           // PATCH: throws ValidationError / NotFoundError
    void remove(TaskId id);                           // throws NotFoundError

    TaskRepository& repository() { return repo_; }

private:
    TaskRepository& repo_;
};

// Exposed for unit tests: normalize (trim) and validate; throw ValidationError listing every problem.
void validate(NewTask& t);
void validate(TaskPatch& p);
void validate(ListQuery& q);

} // namespace tasks
