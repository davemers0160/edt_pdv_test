# ----------------------------------------------------------------------------
# To perform an out of source build do the following on the command line:
# 1. navigate to the root project directory (in this case DfD2)
# 2. mkdir build
# 3. cd build
# 4. depending on your system (windows/linux) type the following
# Example CMake build line for windows & VS2015
# windows: cmake -G "Visual Studio 14 2015 Win64" -DUSE_AVX_INSTRUCTIONS=ON ..
# linux: cmake -DUSE_AVX_INSTRUCTIONS=ON ..
#
# 5. To compile:
# windows: cmake --build . --config Release
# linux: cmake --build . --config Release -- -j4
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)

PROJECT(onnx_project)
set(exe_name "onnx_test")

set(PLATFORM "$ENV{PLATFORM}" CACHE INTERNAL "Copied from environment variable")
if("${PLATFORM}" STREQUAL "")
    set(PLATFORM "NOT_SET")
    message("The PLATFORM Environment variable is not set on this system.  Setting it to:")
endif()
message("Platform: " ${PLATFORM})

# find the root of the main project
string(FIND ${PROJECT_SOURCE_DIR} "/" INDEX REVERSE)
string(SUBSTRING ${PROJECT_SOURCE_DIR} 0 ${INDEX} PR_ROOT)

include(${PR_ROOT}/common/cmake_utils/use_cpp_11.cmake)
include(${PR_ROOT}/common/cmake_utils/use_sse_avx.cmake)

# look for any *.c and *.cpp files
file(GLOB SOURCES "src/*.c*"
    )

set(project_includes 
    "include" 
    "${PR_ROOT}/common/include"
    )

# look for the EDT PDV library
message(" ")
include(${PR_ROOT}/common/cmake_utils/find_edtpdv.cmake)
if(EDTPDV_FOUND)
    set(project_includes ${project_includes} ${EDTPDV_INCLUDE_DIRS})
    set(project_libs ${project_libs} ${EDTPDV_LIBS})
    
    if(UNIX AND NOT APPLE)
        set(project_libs ${project_libs} dl pthread)
        add_compile_options("-lm")
    endif()
endif()


# add a bunch of other libraries
set(project_includes ${project_includes} "${PR_ROOT}/../opencv_helper/select_roi_lib/include")
if(WIN32)
    set(project_libs ${project_libs} "${PR_ROOT}/../opencv_helper/select_roi_lib/build/Release/select_roi.lib")
    set(project_libs ${project_libs} "${PR_ROOT}/ms_tracker_lib/build/Release/ms_tracker.lib")
else()
    set(project_libs ${project_libs} "${PR_ROOT}/../opencv_helper/select_roi_lib/build/libselect_roi.so")
    set(project_libs ${project_libs} "${PR_ROOT}/ms_tracker_lib/build/libms_tracker.so")
endif()


# look for opencv
message(STATUS "--------------------------------------------------------------------------------")
message(STATUS "Looking for OpenCV installation...")
find_package(OpenCV REQUIRED)
if (OpenCV_FOUND)
	set(project_includes ${project_includes} ${OpenCV_INCLUDE_DIRS})
	set(project_libs ${project_libs} ${OpenCV_LIBS})
	message(STATUS "Found OpenCV: " ${OpenCV_DIR})
else()
	message("OpenCV not found...")
endif()
message(STATUS "--------------------------------------------------------------------------------")


message(" ")
message("--------------------------------------------------------------------------------")
message("Source Files: ")
foreach(S ${SOURCES})
    message("  " ${S})
endforeach()
message("--------------------------------------------------------------------------------")

message(" ")

message("--------------------------------------------------------------------------------")
message("Project Includes: ")
foreach(S ${project_includes})
    message("  " ${S})
endforeach()
message("--------------------------------------------------------------------------------")

message(" ")

message("--------------------------------------------------------------------------------")
message("Project Libraries: ")
foreach(S ${project_libs})
    message("  " ${S})
endforeach()
message("--------------------------------------------------------------------------------")

message(" ")

include_directories(${project_includes})
add_executable(${exe_name} ${SOURCES})
target_link_libraries(${exe_name} ${project_libs})

if(WIN32)
    message("MSVC Version: " ${MSVC_VERSION})
    # uncomment for MSVC and cmake greater than 3.6.0
    set(default_build_type "Release")
    if(MSVC AND (${CMAKE_VERSION} VERSION_GREATER "3.6.0"))
        target_compile_options(${exe_name} PRIVATE "/MP")
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${exe_name})
    endif()

endif()

message(" ")
message(STATUS "Build Complete!")
message(" ")
