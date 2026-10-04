# Third-party dependencies, downloaded at configure time with FetchContent.
# Each function is idempotent, so several chapters can call it.
include(FetchContent)

function(clearn_fetch_json)
  if(NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(json
      URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(json)
  endif()
endfunction()

function(clearn_fetch_doctest)
  if(NOT TARGET doctest::doctest)
    FetchContent_Declare(doctest
      GIT_REPOSITORY https://github.com/doctest/doctest.git
      GIT_TAG        v2.4.12
      GIT_SHALLOW    TRUE)
    set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(doctest)
  endif()
endfunction()

# libpqxx (C++ PostgreSQL client), built from source and pinned so every reader
# uses the same API. Needs the PostgreSQL C library libpq installed:
#   macOS: brew install libpq   (+ -DCMAKE_PREFIX_PATH="$(brew --prefix libpq)")
#   Debian/Ubuntu: apt install libpq-dev
function(clearn_fetch_pqxx)
  if(NOT TARGET pqxx)
    set(SKIP_BUILD_TEST ON CACHE BOOL "" FORCE)
    set(BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_Declare(libpqxx
      GIT_REPOSITORY https://github.com/jtv/libpqxx.git
      GIT_TAG        7.10.1
      GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(libpqxx)
  endif()
endfunction()
