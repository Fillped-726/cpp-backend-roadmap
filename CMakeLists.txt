cmake_minimum_required(VERSION 3.25)
project(cpp_backend_roadmap)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg toolchain 已在 CI 或本地指定
find_package(fmt CONFIG REQUIRED)

add_executable(app src/main.cpp)
target_link_libraries(app PRIVATE fmt::fmt)

# 测试
enable_testing()
add_subdirectory(tests)
