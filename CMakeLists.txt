project(ISR)
cmake_minimum_required(VERSION 3.20)

################################
# D435i Compiliation Libraries #
################################

include_directories("C:/Program Files (x86)/Intel RealSense SDK 2.0/include" "C:/Program Files (x86)/Intel RealSense SDK 2.0/third-party" )
link_directories("C:/Program Files (x86)/Intel RealSense SDK 2.0/lib/x64")
add_executable(d435i D435i/main.cpp)
target_link_libraries(d435i realsense2.lib)

################################
# ZED 2 Compiliation Libraries #
################################
option(LINK_SHARED_ZED "Link with the ZED SDK shared executable" ON)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 OLD)
    cmake_policy(SET CMP0015 OLD)
endif(COMMAND cmake_policy)

set(EXECUTABLE_OUTPUT_PATH ".")

find_package(ZED 2 REQUIRED)
find_package(CUDA ${ZED_CUDA_VERSION} EXACT REQUIRED)

IF(NOT WIN32)
    SET(SPECIAL_OS_LIBS "pthread" "X11")
ENDIF()

include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${ZED_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories("C:/OpenCV/opencv/build/include")

link_directories(${ZED_LIBRARY_DIR})
link_directories(${CUDA_LIBRARY_DIRS})

add_executable(zed2_Rec include/utils.hpp ZED/main.cpp)
add_executable(zed2_Parse include/utilsParse.hpp ZED/parse.cpp)
add_definitions(-std=c++14 -O3)

if (LINK_SHARED_ZED)
    set(ZED_LIBS ${ZED_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_CUDART_LIBRARY} ${CUDA_NPP_LIBRARIES_ZED})
else()
    set(ZED_LIBS ${ZED_STATIC_LIBRARIES} ${CUDA_CUDA_LIBRARY} ${CUDA_LIBRARY})
endif()

target_link_libraries(zed2_Rec ${ZED_LIBS} ${OpenCV_LIBRARIES})
target_link_libraries(zed2_Parse ${ZED_LIBS} ${OpenCV_LIBRARIES})

if(INSTALL_SAMPLES)
    list(APPEND SAMPLE_LIST zed2)
    set(SAMPLE_LIST "${SAMPLE_LIST}" PARENT_SCOPE)
endif()

################################
############# END ##############
################################