// Small helpers shared by the chapter 14 programs.
#pragma once

#include <cstdlib>
#include <string>

namespace db {

// Connection string from $DATABASE_URL, or the default used by compose.yaml.
inline std::string database_url() {
    if (const char* env = std::getenv("DATABASE_URL"); env && *env) return env;
    return "postgresql://clearn:clearn@localhost:5432/clearn";
}

} // namespace db
