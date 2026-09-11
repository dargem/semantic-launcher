cmake_minimum_required(VERSION 3.21)

project(semantic-launcher VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(usearch 
    GIT_REPOSITORY https://github.com/unum-cloud/USearch.git
    GIT_TAG main
)
FetchContent_MakeAvailable(usearch)

FetchContent_Declare(llama_cpp
    GIT_REPOSITORY https://github.com/ggerganov/llama.cpp
    GIT_TAG b10830
)
FetchContent_MakeAvailable(llama_cpp)

FetchContent_Declare(rapidfuzz
  GIT_REPOSITORY https://github.com/rapidfuzz/rapidfuzz-cpp.git
  GIT_TAG main)
FetchContent_MakeAvailable(rapidfuzz)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
find_package(LayerShellQt REQUIRED)

qt_standard_project_setup()

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_executable(semantic-launcher
    src/main.cpp
    src/engine/search_engine.cpp
    src/data/database.cpp
    src/embedder/embedder.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/model.gguf ${CMAKE_CURRENT_BINARY_DIR}/model.gguf COPYONLY)

qt_add_qml_module(semantic-launcher
    URI SemanticLauncher
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(semantic-launcher
    PRIVATE Qt6::Quick
    PRIVATE LayerShellQt::Interface
    PRIVATE usearch::usearch
    PRIVATE llama
    PRIVATE rapidfuzz::rapidfuzz
)

install(TARGETS semantic-launcher)