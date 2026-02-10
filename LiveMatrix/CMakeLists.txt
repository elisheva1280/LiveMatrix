cmake_minimum_required(VERSION 3.21)

project(LiveMatrixApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.6 REQUIRED COMPONENTS Quick Qml Multimedia)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/StreamController.cpp
    src/RtspPipeline.cpp
)

set(HEADERS
    include/StreamController.hpp
    include/RtspPipeline.hpp
)

qt_add_executable(LiveMatrix
    WIN32
    ${SOURCES}
    ${HEADERS}
)

qt_add_qml_module(LiveMatrix
    URI LiveMatrixQml
    VERSION 1.0
    QML_FILES
        ui/Main.qml
)

target_include_directories(LiveMatrix
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(LiveMatrix
    PRIVATE
        Qt6::Quick
        Qt6::Qml
        Qt6::Multimedia
        PkgConfig::GSTREAMER
)

if(MSVC)
    target_compile_options(LiveMatrix PRIVATE /W4 /permissive-)
endif()

