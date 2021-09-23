project(ISR)
cmake_minimum_required(VERSION 3.20)

include_directories("C:/Program Files (x86)/Intel RealSense SDK 2.0/include" "C:/Program Files (x86)/Intel RealSense SDK 2.0/third-party" )
link_directories("C:/Program Files (x86)/Intel RealSense SDK 2.0/lib/x64")
add_executable(d435i D435i/main.cpp)
target_link_libraries(d435i realsense2.lib)