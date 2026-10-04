# Expects doctest::doctest to exist (provided by the chapter 12 CMakeLists).
add_executable(fraction_tests test_fraction.cpp)
target_link_libraries(fraction_tests PRIVATE doctest::doctest)
add_test(NAME fraction_tests COMMAND fraction_tests)
