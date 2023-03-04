cmake_minimum_required (VERSION 2.8.11)
project (ProgrammingLanguagePragmatics)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

include_directories(
    re/src
)
add_executable(
  Re_test
  re/test/ReTest.cc
)
target_link_libraries(
    Re_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(Re_test)