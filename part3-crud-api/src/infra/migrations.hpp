// Versioned schema migrations, applied at startup (see chapter 14, example 06).
#pragma once

#include <pqxx/pqxx>

#include "app/logger.hpp"

namespace tasks::infra {

// Applies every migration not yet recorded in schema_migrations. Safe to run
// concurrently from several instances (guarded by an advisory lock).
void run_migrations(pqxx::connection& conn, Logger& logger);

} // namespace tasks::infra
