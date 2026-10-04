cmake_minimum_required(VERSION 3.20)
project(cLearn LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)          # part 5 (C and low-level programming)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # helps editors (clangd) understand the code

option(CLEARN_BUILD_POSTGRES "Build chapter 14 PostgreSQL examples (needs libpqxx)" OFF)
option(CLEARN_BUILD_API      "Build the part 3 CRUD API project"                   OFF)
option(CLEARN_BUILD_C_PART   "Build part 5 (C, hardware, assembly; GCC/Clang only)" ON)

include(cmake/Chapter.cmake)
enable_testing() # lets `ctest --test-dir build` find tests registered in subdirectories

# Every chapter folder becomes a set of executables (one per .cpp).
file(GLOB CHAPTER_DIRS LIST_DIRECTORIES true
     ${CMAKE_SOURCE_DIR}/part1-fundamentals/[0-9]*
     ${CMAKE_SOURCE_DIR}/part2-persistence/[0-9]*)

# Part 5 uses GCC/Clang inline assembly and .S files for x86-64 and AArch64 (not MSVC).
if(CLEARN_BUILD_C_PART AND NOT MSVC AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|arm64|aarch64)$")
  enable_language(ASM)
  file(GLOB PART5_DIRS LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/part5-c-lowlevel/[0-9]*)
  list(APPEND CHAPTER_DIRS ${PART5_DIRS})
endif()

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
