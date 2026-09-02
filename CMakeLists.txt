cmake_minimum_required(VERSION 3.21)

project(semantic-launcher VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
find_package(LayerShellQt REQUIRED)

qt_standard_project_setup()

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_executable(semantic-launcher
    src/main.cpp
    src/engine/search_engine.cpp
    src/engine/search_engine.hpp
)

qt_add_qml_module(semantic-launcher
    URI SemanticLauncher
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(semantic-launcher
    PRIVATE Qt6::Quick
    PRIVATE LayerShellQt::Interface
)

install(TARGETS semantic-launcher)