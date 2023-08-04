project(simulator)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt5 COMPONENTS Core Gui DBus Widgets REQUIRED)

set(COMPILE_DEFINITIONS QT_DEPRECATED_WARNINGS)

set(SOURCES
    main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Widgets
)

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILE_DEFINITIONS})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)