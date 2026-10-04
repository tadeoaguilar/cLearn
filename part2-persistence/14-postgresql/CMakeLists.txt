# Chapter 14 needs libpqxx + libpq, so it's opt-in:
#   cmake -S . -B build -DCLEARN_BUILD_POSTGRES=ON
cmake_minimum_required(VERSION 3.20)
project(Chapter14 LANGUAGES CXX)

if(DEFINED CLEARN_BUILD_POSTGRES AND NOT CLEARN_BUILD_POSTGRES)
  message(STATUS "Chapter 14 skipped (configure with -DCLEARN_BUILD_POSTGRES=ON)")
  return()
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(NOT COMMAND clearn_add_standalone_sources)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Chapter.cmake)
endif()
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Deps.cmake)
clearn_fetch_pqxx()

clearn_add_standalone_sources(${CMAKE_CURRENT_SOURCE_DIR})

# Every executable in this chapter links libpqxx and can include examples/db_common.hpp
get_property(targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(t IN LISTS targets)
  get_target_property(type ${t} TYPE)
  if(type STREQUAL "EXECUTABLE")
    target_link_libraries(${t} PRIVATE pqxx)
    target_include_directories(${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/examples)
  endif()
endforeach()
