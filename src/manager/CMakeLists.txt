project(manager)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# TODO 支持qt6
find_package(Qt5 COMPONENTS Core DBus REQUIRED)

set(COMPILE_DEFINITIONS
    ENABLE_LOAD_HISTORY_TASK
    ENABLE_DBUS_REGISTER
    QT_DEPRECATED_WARNINGS
)

# for test
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND COMPILE_DEFINITIONS
        RANDOM_UPLOAD_STATUS_FAILED
        SIMULATE_UPLOAD_BLOCKED
        RECORD_ALL_TASK_INFO
    )
endif()

# generate dbus adapter file
execute_process(
    COMMAND qdbusxml2cpp io.github.ssk.TaskManager.xml -a taskmanager_adapter
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB SOURCES
    *.cpp
    handler/*.cpp
)
file(GLOB HEADERS
    *.h
    handler/*.h
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/handler
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSETTINGS_QT REQUIRED gsettings-qt)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::DBus
    ${GSETTINGS_QT_LIBRARIES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILE_DEFINITIONS})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
