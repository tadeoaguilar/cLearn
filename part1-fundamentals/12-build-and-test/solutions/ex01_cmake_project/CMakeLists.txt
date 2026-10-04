# Works standalone (cmake -S . -B build) or via add_subdirectory from chapter 12.
cmake_minimum_required(VERSION 3.20)
project(Inventory LANGUAGES CXX)

add_library(inventory src/inventory.cpp)
# PUBLIC: our own sources need the header AND so does anyone linking to `inventory`
target_include_directories(inventory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(inventory PUBLIC cxx_std_20)

add_executable(inventory_app src/main.cpp)
target_link_libraries(inventory_app PRIVATE inventory)
