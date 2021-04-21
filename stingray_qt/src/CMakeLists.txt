cmake_minimum_required(VERSION 3.5)

project(stringray_qt LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

# find the root of the main project
string(FIND ${PROJECT_SOURCE_DIR} "/" INDEX REVERSE)
string(SUBSTRING ${PROJECT_SOURCE_DIR} 0 ${INDEX} PR_ROOT)

message("PR_ROOT:" ${PR_ROOT})

#include(${PR_ROOT}/../../common/cmake_utils/use_cpp_11.cmake)
#include(${PR_ROOT}/../common/cmake_utils/use_sse_avx.cmake)

# look for any *.c and *.cpp files
file(GLOB SOURCES "src/*.c*"
    )

set(project_includes
    "include"
    "${PR_ROOT}/../common/include"
    )


find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        sr_control.cpp
        sr_control.h
        sr_control.ui
)

include_directories(${project_includes})


message(" ")

message("--------------------------------------------------------------------------------")
message("Project Includes: ")
foreach(S ${project_includes})
    message("  " ${S})
endforeach()
message("--------------------------------------------------------------------------------")

message(" ")


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(stringray_qt
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(stringray_qt SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(stringray_qt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(stringray_qt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
