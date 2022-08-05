cmake_minimum_required(VERSION 3.23)

project(bigsort LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bigsortlib src/bigsort.c)
target_include_directories(bigsortlib PUBLIC inc)

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(bigsort_unittest tests/bigsort_unittest.cpp)
target_link_libraries(bigsort_unittest PUBLIC gtest_main bigsortlib)
add_test(NAME bigsort_unittest COMMAND bigsort_unittest)

add_executable(bigsort_test tests/bigsort_test.c)
target_link_libraries(bigsort_test bigsortlib)
add_test(NAME bigsort_test COMMAND bigsort_test)
