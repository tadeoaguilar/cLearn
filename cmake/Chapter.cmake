# Helpers shared by every chapter.
#
# clearn_add_chapter(<chapter_dir>)
#   If the chapter has its own CMakeLists.txt it is added with add_subdirectory()
#   (chapters that build libraries or need extra dependencies). Otherwise every
#   .cpp directly inside examples/ and solutions/ becomes its own executable:
#
#   part1-fundamentals/01-basics/examples/01_hello_world.cpp    -> ch01_ex_01_hello_world
#   part1-fundamentals/01-basics/solutions/ex01_temperature.cpp -> ch01_sol_ex01_temperature
#
# clearn_add_standalone_sources(<chapter_dir>)
#   Only the "one executable per .cpp" part; custom chapter CMakeLists call it too.

find_package(Threads REQUIRED)

function(clearn_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    # GCC wrongly warns when designated initializers skip members that have defaults.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(${target} PRIVATE -Wno-missing-field-initializers)
    endif()
  endif()
endfunction()

function(clearn_add_standalone_sources dir)
  get_filename_component(name ${dir} NAME)  # e.g. 01-basics
  string(SUBSTRING ${name} 0 2 num)          # e.g. 01
  file(RELATIVE_PATH rel ${CMAKE_SOURCE_DIR} ${dir})  # e.g. part1-fundamentals/01-basics
  foreach(kind examples solutions)
    if(kind STREQUAL "examples")
      set(prefix "ch${num}_ex_")
    else()
      set(prefix "ch${num}_sol_")
    endif()
    file(GLOB sources CONFIGURE_DEPENDS ${dir}/${kind}/*.cpp)
    foreach(src IN LISTS sources)
      get_filename_component(stem ${src} NAME_WE)
      set(target ${prefix}${stem})
      add_executable(${target} ${src})
      clearn_warnings(${target})
      target_link_libraries(${target} PRIVATE Threads::Threads)
      # Put binaries next to their chapter: build/part1-fundamentals/01-basics/ch01_ex_...
      set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${rel})
    endforeach()
  endforeach()
endfunction()

function(clearn_add_chapter dir)
  if(EXISTS ${dir}/CMakeLists.txt)
    add_subdirectory(${dir})
  else()
    clearn_add_standalone_sources(${dir})
  endif()
endfunction()
