# Chapter 12 builds a small library, a CLI and unit tests, plus the usual
# one-executable-per-file examples and solutions.
# Works both from the repository root and standalone:
#   cmake -S part1-fundamentals/12-build-and-test -B build-ch12
cmake_minimum_required(VERSION 3.20)
project(Chapter12 LANGUAGES CXX)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(NOT COMMAND clearn_add_standalone_sources)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Chapter.cmake)
endif()

enable_testing()

# doctest: a single-header unit-test framework, downloaded at configure time.
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Deps.cmake)
clearn_fetch_doctest()

add_subdirectory(project)
add_subdirectory(solutions/ex01_cmake_project)
add_subdirectory(solutions/ex02_fraction_tests)

clearn_add_standalone_sources(${CMAKE_CURRENT_SOURCE_DIR})
