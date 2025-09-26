cmake_minimum_required(VERSION 3.10)
project(mini_shell C)             # declare language explicitly if you like

set(EXEC_NAME ${PROJECT_NAME})
set(sources src/mini_shell.c)

# Default to Debug for local development
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # helpful for editors/linters

add_executable(${EXEC_NAME} ${sources})

# include dir
target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Debug/release compile options (target-level, modern CMake)
target_compile_options(${EXEC_NAME} PRIVATE
  $<$<CONFIG:Debug>:-g;-O0;-Wall;-Wextra>
  $<$<CONFIG:Release>:-O3;-Wall>
)

# Example: add address sanitizer for Debug builds (optional)
target_compile_options(${EXEC_NAME} PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<C_COMPILER_ID:GNU>>:-fsanitize=address;-fno-omit-frame-pointer>
)
target_link_options(${EXEC_NAME} PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<C_COMPILER_ID:GNU>>:-fsanitize=address>
)

################################
# Tests (fix and use proper variables)
################################
set(sources_test test/main_test.c)   # set this to your test sources
find_package(GTest QUIET)
if(GTest_FOUND)
  add_executable(unit_tests ${sources_test} ${sources})
  target_compile_definitions(unit_tests PUBLIC UNIT_TESTS)
  target_link_libraries(unit_tests PUBLIC ${GTEST_BOTH_LIBRARIES} pthread)
  target_include_directories(unit_tests PUBLIC ${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()

################################
# Install and CPack
################################
install(TARGETS ${EXEC_NAME} RUNTIME DESTINATION bin)

set(CPACK_PACKAGE_NAME "MiniShell")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_MONOLITHIC_INSTALL 1)
include(CPack)
