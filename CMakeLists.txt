cmake_minimum_required(VERSION 3.21)

project(LiveMatrix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "This project is intended for Windows/MSVC only.")
endif()

find_package(Qt6 6.6 REQUIRED COMPONENTS Quick Qml Multimedia)

qt_standard_project_setup()

# GStreamer 1.22+ via pkg-config (typical Windows SDK install)

set(Qt6QmlTools_FOUND TRUE CACHE BOOL "Faking Qt6QmlTools")
set(Qt6QuickTools_FOUND TRUE CACHE BOOL "Faking Qt6QuickTools")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET
    gstreamer-1.0
    gstreamer-video-1.0
    gstreamer-app-1.0
)

add_subdirectory(LiveMatrix)
