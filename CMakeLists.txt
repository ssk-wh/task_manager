cmake_minimum_required(VERSION 3.7)
project(task_manager)

add_subdirectory(src/manager)
add_subdirectory(src/simulator)
add_subdirectory("tests")

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()