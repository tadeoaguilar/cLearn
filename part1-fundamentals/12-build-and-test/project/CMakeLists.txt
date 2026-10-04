# --- the library: all the logic lives here so it can be tested ---
add_library(textstats src/textstats.cpp)
target_include_directories(textstats PUBLIC include)
target_compile_features(textstats PUBLIC cxx_std_20)
clearn_warnings(textstats)

# --- a thin command-line front end ---
add_executable(textstats_cli app/main.cpp)
target_link_libraries(textstats_cli PRIVATE textstats)
clearn_warnings(textstats_cli)

# --- unit tests ---
add_executable(textstats_tests tests/test_textstats.cpp)
target_link_libraries(textstats_tests PRIVATE textstats doctest::doctest)
add_test(NAME textstats_tests COMMAND textstats_tests)
