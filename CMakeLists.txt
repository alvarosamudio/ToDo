cmake_minimum_required(VERSION 3.14)
project(ToDo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets DBus LinguistTools)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DTK REQUIRED dtk6widget)

set(TS_FILES
    translations/ToDo_es.ts
    translations/ToDo_en.ts
    translations/ToDo_zh.ts
    translations/ToDo_zh_TW.ts
    translations/ToDo_hi.ts
    translations/ToDo_fr.ts
    translations/ToDo_ar.ts
    translations/ToDo_pt.ts
    translations/ToDo_ru.ts
    translations/ToDo_ja.ts
    translations/ToDo_de.ts
    translations/ToDo_it.ts
    translations/ToDo_ko.ts
    translations/ToDo_th.ts
    translations/ToDo_nl.ts
    translations/ToDo_ca.ts
    translations/ToDo_pl.ts
    translations/ToDo_tr.ts
    translations/ToDo_uk.ts
    translations/ToDo_bn.ts
    translations/ToDo_id.ts
    translations/ToDo_ur.ts
    translations/ToDo_pcm.ts
    translations/ToDo_mr.ts
    translations/ToDo_vi.ts
    translations/ToDo_te.ts
    translations/ToDo_sw.ts
    translations/ToDo_ha.ts
)

qt_add_translation(QM_FILES ${TS_FILES})

set(RESOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc")
file(WRITE ${RESOURCE_FILE} "<RCC>\n  <qresource prefix=\"/translations\">\n")
foreach(QM_FILE ${QM_FILES})
    file(RELATIVE_PATH QM_REL "${CMAKE_CURRENT_BINARY_DIR}" "${QM_FILE}")
    file(APPEND ${RESOURCE_FILE} "    <file>${QM_REL}</file>\n")
endforeach()
file(APPEND ${RESOURCE_FILE} "  </qresource>\n</RCC>\n")

add_executable(ToDo
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.ui
    src/MainWindow.h
    src/Task.cpp
    src/Task.ui
    src/Task.h
    resources/imgs.qrc
    ${RESOURCE_FILE}
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
