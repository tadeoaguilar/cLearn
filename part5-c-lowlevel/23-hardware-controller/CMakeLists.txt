# Chapter 23: single-file examples/solutions, plus the simulated FC-1 fan
# controller (firmware + simulator) and its unit tests.
# Works from the repository root and standalone:
#   cmake -S part5-c-lowlevel/23-hardware-controller -B build-ch23
cmake_minimum_required(VERSION 3.20)
project(Chapter23 LANGUAGES C)

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 17)
  set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

if(NOT COMMAND clearn_add_standalone_sources)
  include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Chapter.cmake)
endif()
enable_testing()

set(P ${CMAKE_CURRENT_SOURCE_DIR}/project)

# The controller logic is plain C with no hardware access, so the firmware and the tests share it.
add_library(ch23_controller STATIC ${P}/app/controller.c)
target_include_directories(ch23_controller PUBLIC ${P}/app)
clearn_warnings(ch23_controller)

add_executable(ch23_fan_controller
  ${P}/app/main.c
  ${P}/drivers/gpio.c ${P}/drivers/adc.c ${P}/drivers/pwm.c ${P}/drivers/uart.c ${P}/drivers/timer.c
  ${P}/sim/fc1_sim.c)
target_compile_definitions(ch23_fan_controller PRIVATE FC1_SIMULATOR)
target_include_directories(ch23_fan_controller PRIVATE ${P}/include ${P}/drivers)
target_link_libraries(ch23_fan_controller PRIVATE ch23_controller Threads::Threads)
clearn_warnings(ch23_fan_controller)

add_executable(ch23_controller_tests ${P}/tests/test_controller.c)
target_link_libraries(ch23_controller_tests PRIVATE ch23_controller)
clearn_warnings(ch23_controller_tests)

file(RELATIVE_PATH rel ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
foreach(t ch23_fan_controller ch23_controller_tests)
  set_target_properties(${t} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${rel})
endforeach()

add_test(NAME ch23_controller_tests COMMAND ch23_controller_tests)
add_test(NAME ch23_fan_controller_demo COMMAND ch23_fan_controller --speed 50)
set_tests_properties(ch23_fan_controller_demo PROPERTIES PASS_REGULAR_EXPRESSION "fault acknowledged, back to AUTO")

clearn_add_standalone_sources(${CMAKE_CURRENT_SOURCE_DIR})
