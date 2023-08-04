project(task_tester)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)

enable_testing()
add_executable(task_tester main.cpp)
target_link_libraries(task_tester GTest::GTest GTest::Main)

add_test(NAME task_tester COMMAND task_tester)