find_package(GTest CONFIG REQUIRED)
add_executable(test_main test_main.cpp)
target_link_libraries(test_main PRIVATE GTest::gtest GTest::gtest_main)
add_test(NAME all COMMAND test_main)
