cmake_minimum_required(VERSION 3.20)
project(cLearn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # helps editors (clangd) understand the code

option(CLEARN_BUILD_POSTGRES "Build chapter 14 PostgreSQL examples (needs libpqxx)" OFF)
option(CLEARN_BUILD_API      "Build the part 3 CRUD API project"                   OFF)

include(cmake/Chapter.cmake)
enable_testing() # lets `ctest --test-dir build` find tests registered in subdirectories

# Every chapter folder becomes a set of executables (one per .cpp).
file(GLOB CHAPTER_DIRS LIST_DIRECTORIES true
     ${CMAKE_SOURCE_DIR}/part1-fundamentals/[0-9]*
     ${CMAKE_SOURCE_DIR}/part2-persistence/[0-9]*)

foreach(dir IN LISTS CHAPTER_DIRS)
  if(IS_DIRECTORY ${dir})
    clearn_add_chapter(${dir})
  endif()
endforeach()

if(CLEARN_BUILD_API)
  # The API's PostgreSQL storage follows the chapter-14 switch (both need libpq).
  set(TASKS_WITH_POSTGRES ${CLEARN_BUILD_POSTGRES} CACHE BOOL "" FORCE)
  add_subdirectory(part3-crud-api)
endif()
