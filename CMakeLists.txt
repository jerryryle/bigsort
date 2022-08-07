cmake_minimum_required(VERSION 3.23)

project(bigsort LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sortlib
        src/run.c
        src/merge.c
        )
target_include_directories(sortlib PUBLIC inc)

add_executable(bigsort bigsort.c)
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

add_executable(sortlib_unittest tests/sortlib_unittest.cpp)
target_link_libraries(sortlib_unittest PUBLIC gtest_main sortlib)
add_test(NAME sortlib_unittest COMMAND sortlib_unittest)
