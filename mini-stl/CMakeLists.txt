cmake_minimum_required(VERSION 3.14)
project(mini_vector LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# ---- Catch2 用 FetchContent 拉取 ----
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# ---- 库 ----
add_library(mini_vector INTERFACE)
target_include_directories(mini_vector INTERFACE include)

# ---- 测试 ----
add_executable(test_mini_vector
  test/catch_main.cpp
  test/test_mini_vector.cpp
)
target_link_libraries(test_mini_vector PRIVATE mini_vector Catch2::Catch2WithMain)
enable_testing()
add_test(NAME mini_vector COMMAND test_mini_vector)