# Chapter 13: standalone examples/solutions + two small projects using nlohmann/json.
cmake_minimum_required(VERSION 3.20)
project(Chapter13 LANGUAGES CXX)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(NOT COMMAND clearn_add_standalone_sources)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Chapter.cmake)
endif()
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Deps.cmake)
clearn_fetch_json()

add_executable(ch13_json_example json_example/main.cpp)
target_link_libraries(ch13_json_example PRIVATE nlohmann_json::nlohmann_json)

add_executable(ch13_sol_ex05_json_tasks solutions/ex05_json_tasks/main.cpp)
target_link_libraries(ch13_sol_ex05_json_tasks PRIVATE nlohmann_json::nlohmann_json)

clearn_add_standalone_sources(${CMAKE_CURRENT_SOURCE_DIR})
