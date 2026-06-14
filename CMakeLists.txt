cmake_minimum_required(VERSION 3.14)
project(ToDo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DTK REQUIRED dtk6widget)

add_executable(ToDo
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.ui
    src/MainWindow.h
    src/Task.cpp
    src/Task.ui
    src/Task.h
    resources/imgs.qrc
)

target_include_directories(ToDo PRIVATE
    ${DTK_INCLUDE_DIRS}
)

target_link_libraries(ToDo PRIVATE
    Qt6::Widgets
    Qt6::DBus
    ${DTK_LIBRARIES}
)

target_compile_definitions(ToDo PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
)

install(TARGETS ToDo RUNTIME DESTINATION bin)
install(FILES resources/deepines-todo.desktop DESTINATION share/applications)
install(FILES resources/todo-icon.svg
    DESTINATION share/icons/hicolor/scalable/apps
    RENAME deepines-todo.svg)
install(FILES resources/deepines-todo.appdata.xml DESTINATION share/metainfo)
install(FILES README.md LICENSE DESTINATION share/doc/ToDo)
