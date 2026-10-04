# Tasks API — a REST CRUD service in modern C++.
#
#   cmake -S . -B build && cmake --build build && ./build/tasks_api
#
# Options:
#   TASKS_WITH_POSTGRES  (ON)  build the PostgreSQL storage (needs libpq)
#   TASKS_BUILD_TESTS    (ON)  build the unit/integration tests (ctest)
cmake_minimum_required(VERSION 3.20)
project(TasksApi VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug)
endif()

option(TASKS_WITH_POSTGRES "Build the PostgreSQL repository" ON)
option(TASKS_BUILD_TESTS "Build tests" ON)

find_package(Threads REQUIRED)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake)

function(tasks_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(${target} PRIVATE -Wno-missing-field-initializers)
    endif()
  endif()
endfunction()

# ---------------------------------------------------------------- core library
# Everything that doesn't touch PostgreSQL: domain, service, in-memory storage, HTTP.
add_library(tasks_core
  src/domain/task.cpp
  src/repository/memory_task_repository.cpp
  src/service/task_service.cpp
  src/http/json_mapping.cpp
  src/http/routes.cpp
  src/app/config.cpp
  src/app/logger.cpp)
target_include_directories(tasks_core PUBLIC src)
target_link_libraries(tasks_core PUBLIC nlohmann_json::nlohmann_json httplib::httplib Threads::Threads)
tasks_warnings(tasks_core)

# ---------------------------------------------------------------- postgres library
if(TASKS_WITH_POSTGRES)
  add_library(tasks_pg
    src/infra/connection_pool.cpp
    src/infra/migrations.cpp
    src/repository/pg_task_repository.cpp)
  target_link_libraries(tasks_pg PUBLIC tasks_core pqxx)
  target_compile_definitions(tasks_pg PUBLIC TASKS_WITH_POSTGRES=1)
  tasks_warnings(tasks_pg)
endif()

# ---------------------------------------------------------------- executable
add_executable(tasks_api src/main.cpp)
target_link_libraries(tasks_api PRIVATE tasks_core $<$<BOOL:${TASKS_WITH_POSTGRES}>:tasks_pg>)
tasks_warnings(tasks_api)

# ---------------------------------------------------------------- tests
if(TASKS_BUILD_TESTS)
  enable_testing()
  add_executable(tasks_tests
    tests/main.cpp
    tests/test_domain.cpp
    tests/test_service.cpp
    tests/test_json_mapping.cpp
    tests/test_repository_contract.cpp
    tests/test_http.cpp)
  target_link_libraries(tasks_tests PRIVATE tasks_core doctest::doctest $<$<BOOL:${TASKS_WITH_POSTGRES}>:tasks_pg>)
  tasks_warnings(tasks_tests)
  add_test(NAME tasks_tests COMMAND tasks_tests)
endif()
