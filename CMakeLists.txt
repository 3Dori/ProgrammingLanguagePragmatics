cmake_minimum_required(VERSION 3.9)
project (ProgrammingLanguagePragmatics)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG release-1.11.0)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_subdirectory(RE/src)

include_directories(${PROJECT_SOURCE_DIR}/RE/inc/)
add_executable(
    RETest
    RE/test/RETest.cc
)
target_link_libraries(
    RETest
    GTest::gtest_main
    RE
)
# TODO include-what-you-use

include(GoogleTest)
gtest_discover_tests(RETest)