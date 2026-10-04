# Third-party libraries, pinned and downloaded at configure time.
# Guarded with `if(NOT TARGET ...)` so this project also works inside the cLearn
# super-build, where other chapters may have fetched the same libraries.
include(FetchContent)

# JSON
if(NOT TARGET nlohmann_json::nlohmann_json)
  FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(json)
endif()

# HTTP server/client (header-only). We keep it dependency-free: no TLS/compression.
if(NOT TARGET httplib::httplib)
  set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
  set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
  set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
  set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.20.1
    GIT_SHALLOW    TRUE)
  FetchContent_MakeAvailable(httplib)
endif()

# PostgreSQL C++ client (needs libpq: `brew install libpq` / `apt install libpq-dev`)
if(TASKS_WITH_POSTGRES AND NOT TARGET pqxx)
  set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
  set(BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF)
  FetchContent_Declare(libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG        7.10.1
    GIT_SHALLOW    TRUE)
  FetchContent_MakeAvailable(libpqxx)
endif()

# Unit testing
if(TASKS_BUILD_TESTS AND NOT TARGET doctest::doctest)
  set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
  FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.12
    GIT_SHALLOW    TRUE)
  FetchContent_MakeAvailable(doctest)
endif()
