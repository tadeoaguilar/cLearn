// Domain errors: what can go wrong, independent of HTTP or SQL.
// The HTTP layer maps them to status codes (see http/routes.cpp).
#pragma once

#include <map>
#include <stdexcept>
#include <string>

namespace tasks {

// Input breaks a business rule. `fields` maps field name -> problem.
class ValidationError : public std::runtime_error {
public:
    explicit ValidationError(std::map<std::string, std::string> fields)
        : std::runtime_error("validation failed"), fields_{std::move(fields)} {}
    ValidationError(const std::string& field, const std::string& problem)
        : ValidationError(std::map<std::string, std::string>{{field, problem}}) {}

    const std::map<std::string, std::string>& fields() const { return fields_; }

private:
    std::map<std::string, std::string> fields_;
};

// The requested entity doesn't exist.
class NotFoundError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Storage is temporarily unreachable (database down, pool exhausted).
class StorageUnavailableError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

} // namespace tasks
