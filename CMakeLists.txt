cmake_minimum_required(VERSION 3.23)

project(bigsort LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sortlib
        src/bigsort.c
        src/merge.c
        src/round.c
        src/run.c
        )
target_include_directories(sortlib PUBLIC src)

add_executable(bigsort src/main.c)
target_link_libraries(bigsort sortlib)

# -----------------------------------------------------------------------------
# Download googletest and make it available to the project. This was copied
# from the instructions here:
# https://github.com/google/googletest/blob/main/googletest/README.md
#
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# -----------------------------------------------------------------------------

enable_testing()

add_executable(unit_tests
        tests/round_test.cpp
        )
target_link_libraries(unit_tests PUBLIC gtest_main sortlib)
add_test(
        NAME unit_tests
        COMMAND unit_tests)

find_package(Python COMPONENTS Interpreter REQUIRED)

add_test(
        NAME integration_tests
        COMMAND ${Python_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/tests_integration
)
set_property(TEST integration_tests PROPERTY ENVIRONMENT "BIGSORT_PATH=$<TARGET_FILE:bigsort>")
